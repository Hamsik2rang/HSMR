#include "Renderer/RenderResourceManager.h"
#include "Renderer/RenderDefinition.h"
#include "Renderer/RendererDefinition.h"
#include "Renderer/ShaderLibrary.h"

#include "Core/Log.h"
#include "Core/Hash.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"

#include "Resource/Material.h"
#include "Resource/Mesh.h"
#include "Resource/Shader.h"
#include "Resource/Image.h"

#include "Renderer/CameraUtils.h"

// ECS Scene support
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components/Components.h"

HS_NS_BEGIN

RenderResourceManager::RenderResourceManager(RHIContext* rhiContext)
    : _rhiContext(rhiContext)
{
}

RenderResourceManager::~RenderResourceManager()
{
    ReleaseAll();
}

CameraResource* RenderResourceManager::GetOrCreateCameraResource(uint64 resourceKey, const PerView& perView)
{
    auto it = _cameraResources.find(resourceKey);
    if (it != _cameraResources.end() && it->second.isValid)
    {
        auto* resource = &it->second;

        // TODO: UpdateBuffer는 CPU-GPU 동기화 이슈가 있을 수 있으니, 실제로는 더 효율적인 업데이트 전략이 필요할 수 있다 (예: Persistent Mapped Buffer + Ring Buffer)
        // TODO: cameraComponent이 자주 변경되지 않는다면, 매 프레임 업데이트하는 대신 변경된 경우에만 업데이트하도록 최적화할 수 있다 (예: CameraComponent에 dirty flag 추가)
        if (memcmp(&resource->perViewData, &perView, sizeof(PerView)) != 0)
        {
            resource->perViewData = perView;
            _rhiContext->UpdateBuffer(resource->perViewBuffer, 0, &resource->perViewData, sizeof(PerView));
        }
    }
    else
    {
        CameraResource resource{};

        resource.perViewData = perView;
        resource.perViewBuffer = _rhiContext->CreateBuffer(
            "PerView UBO", &resource.perViewData, sizeof(PerView),
            EBufferUsage::Uniform, EBufferMemoryOption::Dynamic
        );

        if (!resource.perViewBuffer)
        {
            HS_LOG(error, "[RenderResourceManager] Failed to create PerView buffer");
            return nullptr;
        }

        resource.isValid             = true;
        _cameraResources[resourceKey] = std::move(resource);

        HS_LOG(info, "[RenderResourceManager] CameraResource created");
    }
    return &_cameraResources[resourceKey];
}

LightResource* RenderResourceManager::GetOrCreateLightResource(uint64 resourceKey, const LightUBO& lightData)
{
    bool created = false;

    auto it = _lightResources.find(resourceKey);

    if (it == _lightResources.end())
    {
        LightResource resource{};
        _lightResources[resourceKey] = resource;
        created                      = true;
    }
    LightResource* resource = &_lightResources[resourceKey];

    if (created)
    {
        resource->lightData = lightData;
        resource->lightBuffer = _rhiContext->CreateBuffer(
            "Light Buffer",
            &resource->lightData,
            sizeof(LightUBO),
            EBufferUsage::Uniform,
            EBufferMemoryOption::Dynamic
        );
        resource->isValid = resource->lightBuffer != nullptr;
    }
    else
    {
        // TODO: UpdateBuffer는 CPU-GPU 동기화 이슈가 있을 수 있으니, 실제로는 더 효율적인 업데이트 전략이 필요할 수 있다 (예: Persistent Mapped Buffer + Ring Buffer)
        // TODO: light이 자주 변경되지 않는다면, 매 프레임 업데이트하는 대신 변경된 경우에만 업데이트하도록 최적화할 수 있다 (예: LightComponent에 dirty flag 추가)
        if (memcmp(&resource->lightData, &lightData, sizeof(LightUBO)) != 0)
        {
            resource->lightData = lightData;
            _rhiContext->UpdateBuffer(resource->lightBuffer, 0, &resource->lightData, sizeof(LightUBO));
        }
    }

    if (!resource->isValid)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create Light buffer");
        return nullptr;
    }

    return &_lightResources[resourceKey];
}

RHIBuffer* RenderResourceManager::getOrCreatePerDrawBuffer(uint64 resourceKey, const glm::mat4& worldMatrix, uint32 worldVersion)
{
    RHIBuffer* buffer = nullptr;

    PerDraw perDraw{
        .modelMatrix        = worldMatrix,
        .inverseModelMatrix = glm::inverse(worldMatrix)
    };

    auto it = _perDrawBuffers.find(resourceKey);
    if (it == _perDrawBuffers.end())
    {
        buffer = _rhiContext->CreateBuffer(
            "PerDraw UBO", &perDraw, sizeof(PerDraw),
            EBufferUsage::Uniform, EBufferMemoryOption::Dynamic
        );
        _perDrawBuffers[resourceKey]        = buffer;
        _perDrawBufferVersions[resourceKey] = worldVersion;
    }
    else
    {
        // TODO: UpdateBuffer는 CPU-GPU 동기화 이슈가 있을 수 있으니, 실제로는 더 효율적인 업데이트 전략이 필요할 수 있다 (예: Persistent Mapped Buffer + Ring Buffer)
        // TODO: transform이 자주 변경되지 않는다면, 매 프레임 업데이트하는 대신 변경된 경우에만 업데이트하도록 최적화할 수 있다 (예: TransformComponent에 dirty flag 추가)
        // 현재는 간단히 매 호출마다 업데이트하도록 구현
        buffer = _perDrawBuffers[resourceKey];
        if (_perDrawBufferVersions[resourceKey] != worldVersion)
        {
            _rhiContext->UpdateBuffer(buffer, 0, &perDraw, sizeof(PerDraw));
            _perDrawBufferVersions[resourceKey] = worldVersion;
        }
    }

    if (!buffer)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create PerDraw buffer");
        return nullptr;
    }

    return buffer;
}

SceneResource RenderResourceManager::BuildSceneResource(
    Scene* scene,
    ShaderLibrary* shaderLibrary
)
{
    SceneResource sceneResource;

    if (!scene) return sceneResource;

    auto& registry   = scene->GetRegistry();
    bool vulkanYFlip = (_rhiContext->GetCurrentPlatform() == ERHIPlatform::Vulkan);

    // 1. Cameras: CameraComponent + TransformComponent → CameraResource
    auto cameraView    = registry.view<TransformComponent, CameraComponent>();
    uint32 cameraIndex = 0;
    for (auto [entity, transform, camera] : cameraView.each())
    {
        PerView perView        = CameraUtils::BuildPerViewData(transform, camera, vulkanYFlip);
        CameraResource* camRes = GetOrCreateCameraResource(static_cast<uint64>(entt::to_integral(entity)), perView);
        if (camRes)
        {
            sceneResource.cameraResources.push_back(camRes);
        }
        ++cameraIndex;
    }

    // 2. Lights (TODO: Scene LightComponent → LightResource)
    auto lightView = registry.view<TransformComponent, LightComponent>();
    for (auto [entity, transform, light] : lightView.each())
    {
        LightUBO lightUBO{};
        lightUBO.position  = glm::vec4(transform.GetWorldPosition(), 0.0f);
        lightUBO.color     = light.color;
        lightUBO.intensity = light.intensity;
        lightUBO.direction = glm::normalize(glm::mat3(transform.worldMatrix) * glm::vec3(0.0f, 0.0f, -1.0f));
        lightUBO.type      = static_cast<int>(light.type);

        LightResource* lightRes = GetOrCreateLightResource(static_cast<uint64>(entt::to_integral(entity)), lightUBO);

        sceneResource.lightResources.push_back(lightRes);
    }

    // 3. MeshRenderer entities → RenderModel
    auto meshView = registry.view<TransformComponent, MeshRendererComponent>();
    for (auto [entity, transform, meshRenderer] : meshView.each())
    {
        if (!meshRenderer.IsValidForRendering()) continue;

        Mesh* mesh    = meshRenderer.mesh;
        Material* mat = meshRenderer.GetMaterial(0);
        if (!mesh || !mat) continue;

        // Assign default shader if needed
        Shader* shader = mat->GetShader();
        if ((!shader || !shader->IsCompiledEx()) && shaderLibrary)
        {
            Shader* defaultShader = shaderLibrary->GetOrCompile("BlinnPhong");
            if (defaultShader)
            {
                mat->SetShader(defaultShader);
                shader = defaultShader;
            }
        }

        if (!shader || !shader->IsCompiledEx()) continue;

        const ShaderReflectionDataEx& reflection = shader->GetReflection();

        RHIBuffer* perDrawBuffer = getOrCreatePerDrawBuffer(
            static_cast<uint64>(entt::to_integral(entity)),
            transform.worldMatrix,
            transform.worldVersion
        );
        if (!perDrawBuffer) continue;

        MaterialResource* matRes = GetOrCreateMaterialResources(mat);
        if (!matRes) continue;

        DrawResource* drawRes = getOrCreateDrawResource(
            static_cast<uint64>(entt::to_integral(entity)),
            mat,
            matRes,
            sceneResource.cameraResources.empty() ? nullptr : sceneResource.cameraResources[0],
            perDrawBuffer,
            sceneResource.lightResources.empty() ? nullptr : sceneResource.lightResources[0]
        );
        if (!drawRes) continue;

        MeshResource* meshRes = GetOrCreateMeshResources(mesh, reflection);
        if (!meshRes) continue;

        RenderModel renderModel;
        renderModel.worldMatrix        = transform.worldMatrix;
        renderModel.inverseWorldMatrix = glm::inverse(renderModel.worldMatrix);
        renderModel.material           = mat;
        renderModel.perDrawBuffer      = perDrawBuffer;
        renderModel.meshResource       = meshRes;
        renderModel.materialResource   = matRes;
        renderModel.drawResource       = drawRes;
        sceneResource.renderModels.push_back(renderModel);
    }

    return sceneResource;
}

RenderSceneSnapshot RenderResourceManager::BuildRenderSceneSnapshot(Scene* scene, ShaderLibrary* shaderLibrary)
{
    RenderSceneSnapshot snapshot;

    if (!scene) return snapshot;

    auto& registry = scene->GetRegistry();
    bool vulkanYFlip = (_rhiContext->GetCurrentPlatform() == ERHIPlatform::Vulkan);

    auto cameraView = registry.view<TransformComponent, CameraComponent>();
    for (auto [entity, transform, camera] : cameraView.each())
    {
        RenderViewSnapshot viewSnapshot{};
        viewSnapshot.viewId  = static_cast<uint64>(entt::to_integral(entity));
        viewSnapshot.perView = CameraUtils::BuildPerViewData(transform, camera, vulkanYFlip);
        snapshot.views.push_back(viewSnapshot);
    }

    auto lightView = registry.view<TransformComponent, LightComponent>();
    for (auto [entity, transform, light] : lightView.each())
    {
        RenderLightSnapshot lightSnapshot{};
        lightSnapshot.lightId = static_cast<uint64>(entt::to_integral(entity));
        lightSnapshot.light.position  = glm::vec4(transform.GetWorldPosition(), 0.0f);
        lightSnapshot.light.color     = light.color;
        lightSnapshot.light.intensity = light.intensity;
        lightSnapshot.light.direction = glm::normalize(glm::mat3(transform.worldMatrix) * glm::vec3(0.0f, 0.0f, -1.0f));
        lightSnapshot.light.type      = static_cast<int>(light.type);
        snapshot.lights.push_back(lightSnapshot);
    }

    auto meshView = registry.view<TransformComponent, MeshRendererComponent>();
    for (auto [entity, transform, meshRenderer] : meshView.each())
    {
        if (!meshRenderer.IsValidForRendering()) continue;

        Mesh* mesh = meshRenderer.mesh;
        Material* mat = meshRenderer.GetMaterial(0);
        if (!mesh || !mat) continue;

        Shader* shader = mat->GetShader();
        if ((!shader || !shader->IsCompiledEx()) && shaderLibrary)
        {
            Shader* defaultShader = shaderLibrary->GetOrCompile("BlinnPhong");
            if (defaultShader)
            {
                mat->SetShader(defaultShader);
                shader = defaultShader;
            }
        }

        if (!shader || !shader->IsCompiledEx()) continue;

        RenderPrimitiveSnapshot primitiveSnapshot{};
        primitiveSnapshot.primitiveId  = static_cast<uint64>(entt::to_integral(entity));
        primitiveSnapshot.worldVersion = transform.worldVersion;
        primitiveSnapshot.worldMatrix  = transform.worldMatrix;
        primitiveSnapshot.mesh         = mesh;
        primitiveSnapshot.material     = mat;
        snapshot.primitives.push_back(primitiveSnapshot);
    }

    return snapshot;
}

SceneResource RenderResourceManager::BuildSceneResource(const RenderSceneSnapshot& snapshot)
{
    SceneResource sceneResource;

    for (const RenderViewSnapshot& viewSnapshot : snapshot.views)
    {
        CameraResource* camRes = GetOrCreateCameraResource(viewSnapshot.viewId, viewSnapshot.perView);
        if (camRes)
        {
            sceneResource.cameraResources.push_back(camRes);
        }
    }

    for (const RenderLightSnapshot& lightSnapshot : snapshot.lights)
    {
        LightResource* lightRes = GetOrCreateLightResource(lightSnapshot.lightId, lightSnapshot.light);
        if (lightRes)
        {
            sceneResource.lightResources.push_back(lightRes);
        }
    }

    CameraResource* cameraResource = sceneResource.cameraResources.empty() ? nullptr : sceneResource.cameraResources[0];
    LightResource* lightResource = sceneResource.lightResources.empty() ? nullptr : sceneResource.lightResources[0];

    for (const RenderPrimitiveSnapshot& primitive : snapshot.primitives)
    {
        Mesh* mesh = primitive.mesh;
        Material* mat = primitive.material;
        if (!mesh || !mat) continue;

        Shader* shader = mat->GetShader();
        if (!shader || !shader->IsCompiledEx()) continue;

        const ShaderReflectionDataEx& reflection = shader->GetReflection();

        RHIBuffer* perDrawBuffer = getOrCreatePerDrawBuffer(
            primitive.primitiveId,
            primitive.worldMatrix,
            primitive.worldVersion
        );
        if (!perDrawBuffer) continue;

        MaterialResource* matRes = GetOrCreateMaterialResources(mat);
        if (!matRes) continue;

        DrawResource* drawRes = getOrCreateDrawResource(
            primitive.primitiveId,
            mat,
            matRes,
            cameraResource,
            perDrawBuffer,
            lightResource
        );
        if (!drawRes) continue;

        MeshResource* meshRes = GetOrCreateMeshResources(mesh, reflection);
        if (!meshRes) continue;

        RenderModel renderModel;
        renderModel.worldMatrix        = primitive.worldMatrix;
        renderModel.inverseWorldMatrix = glm::inverse(renderModel.worldMatrix);
        renderModel.material           = mat;
        renderModel.perDrawBuffer      = perDrawBuffer;
        renderModel.meshResource       = meshRes;
        renderModel.materialResource   = matRes;
        renderModel.drawResource       = drawRes;
        sceneResource.renderModels.push_back(renderModel);
    }

    return sceneResource;
}

size_t RenderResourceManager::buildDrawResourceKey(
    uint64 primitiveId,
    Material* material,
    CameraResource* cameraResource,
    RHIBuffer* perDrawBuffer,
    LightResource* lightResource
) const
{
    uint64 key = HashCombine64(primitiveId, reinterpret_cast<uint64>(material));
    key = HashCombine64(key, reinterpret_cast<uint64>(cameraResource ? cameraResource->perViewBuffer : nullptr));
    key = HashCombine64(key, reinterpret_cast<uint64>(perDrawBuffer));
    key = HashCombine64(key, reinterpret_cast<uint64>(lightResource ? lightResource->lightBuffer : nullptr));
    return static_cast<size_t>(key);
}

DrawResource* RenderResourceManager::getOrCreateDrawResource(
    uint64 primitiveId,
    Material* material,
    MaterialResource* materialResource,
    CameraResource* cameraResource,
    RHIBuffer* perDrawBuffer,
    LightResource* lightResource
)
{
    if (!material || !materialResource || !perDrawBuffer)
    {
        return nullptr;
    }

    size_t resourceKey = buildDrawResourceKey(primitiveId, material, cameraResource, perDrawBuffer, lightResource);
    auto it = _drawResources.find(resourceKey);
    if (it != _drawResources.end() && it->second.isValid)
    {
        return &it->second;
    }

    Shader* shader = material->GetShader();
    if (!shader || !shader->IsCompiledEx())
    {
        return nullptr;
    }

    DrawResource resource{};
    resource.resourceLayout = createResourceLayoutFromReflection(
        shader->GetReflection(),
        material,
        cameraResource,
        perDrawBuffer,
        lightResource
    );
    if (!resource.resourceLayout)
    {
        return nullptr;
    }

    resource.resourceSet = _rhiContext->CreateResourceSet("DrawResourceSet", resource.resourceLayout);
    if (!resource.resourceSet)
    {
        _rhiContext->DestroyResourceLayout(resource.resourceLayout);
        return nullptr;
    }

    resource.isValid = true;
    _drawResources[resourceKey] = std::move(resource);
    return &_drawResources[resourceKey];
}

MaterialResource* RenderResourceManager::GetOrCreateMaterialResources(Material* material)
{
    auto it = _materialResources.find(material);
    if (it != _materialResources.end() && it->second.isValid)
    {
        return &it->second;
    }

    MaterialResource resources = createMaterialResources(material);
    if (resources.isValid)
    {
        _materialResources[material] = std::move(resources);
        return &_materialResources[material];
    }

    return nullptr;
}

RHIGraphicsPipeline* RenderResourceManager::GetOrCreatePipeline(Material* material, RHIRenderPass* renderPass)
{
    if (!renderPass)
    {
        return nullptr;
    }

    return GetOrCreatePipeline(material, MakePipelineRenderTargetLayout(renderPass->info));
}

RHIGraphicsPipeline* RenderResourceManager::GetOrCreatePipeline(Material* material, const PipelineRenderTargetLayout& renderTargetLayout)
{
    MaterialResource* matRes = GetOrCreateMaterialResources(material);
    if (!matRes) return nullptr;

    size_t pipelineKey = std::hash<PipelineRenderTargetLayout>{}(renderTargetLayout);
    pipelineKey = HashCombine64(pipelineKey, static_cast<uint32>(_rhiContext->GetCapabilities().resourceBindingTier));

    auto it = matRes->pipelineCache.find(pipelineKey);
    if (it != matRes->pipelineCache.end())
    {
        return it->second;
    }

    Shader* shader = material->GetShader();
    if (!shader || !shader->IsCompiledEx()) return nullptr;

    const ShaderReflectionDataEx& reflection = shader->GetReflection();

    // Build vertex input descriptor from reflection
    VertexInputStateDescriptor viDesc{};
    VertexInputLayoutDescriptor viLayout{};
    viLayout.binding       = 0;
    viLayout.stride        = reflection.vertexInput.stride;
    viLayout.stepRate      = 1;
    viLayout.useInstancing = false;
    viDesc.layouts.push_back(viLayout);

    for (const auto& attr : reflection.vertexInput.attributes)
    {
        VertexInputAttributeDescriptor viAttr{};
        viAttr.location = attr.location;
        viAttr.binding  = 0;
        viAttr.format   = attr.format;
        viAttr.offset   = attr.offset;
        viDesc.attributes.push_back(viAttr);
    }

    // Standard pipeline settings
    DepthStencilStateDescriptor dsDesc{};
    dsDesc.depthTestEnable  = true;
    dsDesc.depthWriteEnable = true;
    dsDesc.depthCompareOp   = ECompareOp::Less;

    ColorBlendStateDescriptor cbDesc{};
    cbDesc.attachmentCount = renderTargetLayout.colorAttachmentCount;
    cbDesc.attachments.resize(cbDesc.attachmentCount);
    for (size_t i = 0; i < cbDesc.attachmentCount; ++i)
    {
        cbDesc.attachments[i].blendEnable = false;
    }

    RasterizerStateDescriptor rsDesc{};
    rsDesc.cullMode                = material->IsTwoSided() ? ECullMode::None : ECullMode::Back;
    rsDesc.frontFace               = EFrontFace::CounterClockwise;
    rsDesc.polygonMode             = EPolygonMode::Fill;
    rsDesc.depthClampEnable        = false;
    rsDesc.rasterizerDiscardEnable = false;
    rsDesc.depthBiasEnable         = false;

    ShaderProgramDescriptor spDesc{};
    spDesc.stages.resize(2);
    spDesc.stages[0] = matRes->vertexShader;
    spDesc.stages[1] = matRes->fragmentShader;

    InputAssemblyStateDescriptor iaDesc{};
    iaDesc.primitiveTopology = EPrimitiveTopology::TriangleList;

    GraphicsPipelineInfo gpInfo{};
    gpInfo.shaderDesc        = spDesc;
    gpInfo.inputAssemblyDesc = iaDesc;
    gpInfo.vertexInputDesc   = viDesc;
    gpInfo.rasterizerDesc    = rsDesc;
    gpInfo.depthStencilDesc  = dsDesc;
    gpInfo.colorBlendDesc    = cbDesc;
    gpInfo.renderPass        = nullptr;
    gpInfo.renderTargetLayout = renderTargetLayout;
    gpInfo.resourceLayout    = matRes->resourceLayout;

    RHIGraphicsPipeline* pipeline = _rhiContext->CreateGraphicsPipeline("AutoPipeline", gpInfo);
    if (pipeline)
    {
        matRes->pipelineCache[pipelineKey] = pipeline;
    }

    return pipeline;
}

MeshResource* RenderResourceManager::GetOrCreateMeshResources(Mesh* mesh, const ShaderReflectionDataEx& reflection)
{
    auto it = _meshResources.find(mesh);
    if (it != _meshResources.end() && it->second.isValid)
    {
        return &it->second;
    }

    const auto& positions = mesh->GetPosition();
    const auto& indices   = mesh->GetIndices();
    if (positions.empty() || indices.empty()) return nullptr;

    MeshResource resources;

    // Build interleaved vertex data based on reflection
    std::vector<float> interleavedData = buildInterleavedVertexData(mesh, reflection.vertexInput);

    if (interleavedData.empty()) return nullptr;

    resources.vertexBuffer = _rhiContext->CreateBuffer(
        "Mesh VB", interleavedData.data(),
        interleavedData.size() * sizeof(float),
        EBufferUsage::Vertex, EBufferMemoryOption::Mapped
    );

    resources.indexBuffer = _rhiContext->CreateBuffer(
        "Mesh IB", indices.data(),
        indices.size() * sizeof(uint32),
        EBufferUsage::Index, EBufferMemoryOption::Mapped
    );

    resources.indexCount = static_cast<uint32>(indices.size());
    resources.isValid    = true;

    _meshResources[mesh] = std::move(resources);
    return &_meshResources[mesh];
}

ImageResource* RenderResourceManager::GetOrCreateImageResource(Image* image)
{
    if (!image) return nullptr;

    auto it = _imageResources.find(image);
    if (it != _imageResources.end() && it->second.isValid)
    {
        return &it->second;
    }

    ImageResource resource = createImageResource(image);
    if (resource.isValid)
    {
        _imageResources[image] = std::move(resource);
        HS_LOG(info, "[RenderResourceManager] ImageResource created (%ux%u)", _imageResources[image].width, _imageResources[image].height);
        return &_imageResources[image];
    }

    return nullptr;
}

ImageResource RenderResourceManager::createImageResource(Image* image)
{
    ImageResource resource;

    if (!image || !image->GetRawData())
    {
        HS_LOG(error, "[RenderResourceManager] Invalid image data");
        return resource;
    }

    uint32 width   = image->GetWidth();
    uint32 height  = image->GetHeight();
    uint8 channels = image->GetChannel();

    // Determine pixel format
    EPixelFormat format = EPixelFormat::R8G8B8A8Unorm;
    if (channels == 1)
        format = EPixelFormat::R8Unorm;
    else if (channels == 2)
        format = EPixelFormat::RG8Unorm;
    // 3 and 4 channels both use RGBA (RGB8 is inefficient in Vulkan)

    // Prepare image data (convert RGB to RGBA if needed)
    const void* imageData = image->GetRawData();
    std::vector<uint8> rgbaData;
    size_t dataSize = 0;

    if (channels == 3)
    {
        // Convert RGB to RGBA
        rgbaData.resize(width * height * 4);
        const uint8* src = static_cast<const uint8*>(imageData);
        for (uint32 i = 0; i < width * height; ++i)
        {
            rgbaData[i * 4 + 0] = src[i * 3 + 0];
            rgbaData[i * 4 + 1] = src[i * 3 + 1];
            rgbaData[i * 4 + 2] = src[i * 3 + 2];
            rgbaData[i * 4 + 3] = 255;
        }
        imageData = rgbaData.data();
        dataSize  = rgbaData.size();
    }
    else
    {
        dataSize = width * height * channels;
    }

    // Create RHITexture
    TextureInfo texInfo{};
    texInfo.format        = format;
    texInfo.type          = ETextureType::Tex2D;
    texInfo.usage         = ETextureUsage::Sampled | ETextureUsage::Static; // STATIC adds TRANSFER_DST for data upload
    texInfo.extent.width  = width;
    texInfo.extent.height = height;
    texInfo.extent.depth  = 1;
    texInfo.mipLevel      = 1;
    texInfo.arrayLength   = 1;
    texInfo.byteSize      = dataSize;

    resource.texture = _rhiContext->CreateTexture(
        "ImageTex", const_cast<void*>(imageData), texInfo
    );

    if (!resource.texture)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create RHITexture");
        return resource;
    }

    // Create RHISampler
    SamplerInfo sampInfo{};
    sampInfo.type       = ETextureType::Tex2D;
    sampInfo.minFilter  = EFilterMode::Linear;
    sampInfo.magFilter  = EFilterMode::Linear;
    sampInfo.mipmapMode = EFilterMode::Linear;
    sampInfo.addressU   = EAddressMode::Repeat;
    sampInfo.addressV   = EAddressMode::Repeat;
    sampInfo.addressW   = EAddressMode::Repeat;

    resource.sampler = _rhiContext->CreateSampler("ImageSampler", sampInfo);

    if (!resource.sampler)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create RHISampler");
        _rhiContext->DestroyTexture(resource.texture);
        resource.texture = nullptr;
        return resource;
    }

    resource.width   = width;
    resource.height  = height;
    resource.format  = format;
    resource.isValid = true;

    return resource;
}

MaterialResource RenderResourceManager::createMaterialResources(Material* material)
{
    MaterialResource resources;

    Shader* shader = material->GetShader();
    if (!shader || !shader->IsCompiledEx())
    {
        HS_LOG(error, "[RenderResourceManager] Shader not compiled for material");
        return resources;
    }

    const ShaderReflectionDataEx& reflection = shader->GetReflection();

    // Debug: Log reflection info
    HS_LOG(info, "[RenderResourceManager] Shader '%s' reflection: %zu buffers, %zu textures, %zu samplers", shader->GetShaderName().c_str(), reflection.bufferBindings.size(), reflection.textureBindings.size(), reflection.samplerBindings.size());

    for (const auto& tex : reflection.textureBindings)
    {
        HS_LOG(info, "[RenderResourceManager]   Texture: '%s' binding=%u set=%u", tex.name.c_str(), tex.binding, tex.set);
    }

    // Create RHI shaders from bytecode
    const auto* vsBytecode = shader->GetBytecode(EShaderStage::Vertex);
    const auto* fsBytecode = shader->GetBytecode(EShaderStage::Fragment);

    if (!vsBytecode || !fsBytecode)
    {
        HS_LOG(error, "[RenderResourceManager] Missing vertex or fragment bytecode");
        return resources;
    }

    ShaderInfo vsInfo{};
    vsInfo.stage           = EShaderStage::Vertex;
    vsInfo.entryName       = shader->GetEntryPoint(EShaderStage::Vertex);
    resources.vertexShader = _rhiContext->CreateShader(
        "VS", vsInfo,
        reinterpret_cast<const char*>(vsBytecode->data()),
        vsBytecode->size()
    );

    ShaderInfo fsInfo{};
    fsInfo.stage             = EShaderStage::Fragment;
    fsInfo.entryName         = shader->GetEntryPoint(EShaderStage::Fragment);
    resources.fragmentShader = _rhiContext->CreateShader(
        "FS", fsInfo,
        reinterpret_cast<const char*>(fsBytecode->data()),
        fsBytecode->size()
    );

    if (!resources.vertexShader || !resources.fragmentShader)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create RHI shaders");
        return resources;
    }

    resources.resourceLayout = createResourceLayoutFromReflection(reflection, material, nullptr, nullptr, nullptr);

    if (!resources.resourceLayout)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create resource layout");
        return resources;
    }

    resources.isValid = true;
    HS_LOG(info, "[RenderResourceManager] Material resources created for shader '%s'", shader->GetShaderName().c_str());
    return resources;
}

// Helper function to map shader texture name to material texture type
static EMaterialTextureType mapTextureNameToType(const std::string& name)
{
    // Convert to lowercase for comparison
    std::string lowerName = name;
    for (auto& c : lowerName) c = static_cast<char>(tolower(c));

    if (lowerName.find("albedo") != std::string::npos ||
        lowerName.find("diffuse") != std::string::npos ||
        lowerName.find("basecolor") != std::string::npos ||
        lowerName.find("base_color") != std::string::npos)
    {
        return EMaterialTextureType::Diffuse;
    }

    if (lowerName.find("normal") != std::string::npos)
    {
        return EMaterialTextureType::Normal;
    }

    if (lowerName.find("metallic") != std::string::npos ||
        lowerName.find("metalness") != std::string::npos)
    {
        return EMaterialTextureType::Metallic;
    }

    if (lowerName.find("roughness") != std::string::npos)
    {
        return EMaterialTextureType::Roughness;
    }

    if (lowerName.find("emission") != std::string::npos ||
        lowerName.find("emissive") != std::string::npos)
    {
        return EMaterialTextureType::Emission;
    }

    if (lowerName.find("ao") != std::string::npos ||
        lowerName.find("occlusion") != std::string::npos ||
        lowerName.find("ambient") != std::string::npos)
    {
        return EMaterialTextureType::AmbientOcclusion;
    }

    if (lowerName.find("specular") != std::string::npos)
    {
        return EMaterialTextureType::Specular;
    }

    // Default to diffuse
    return EMaterialTextureType::Diffuse;
}

RHIResourceLayout* RenderResourceManager::createResourceLayoutFromReflection(
    const ShaderReflectionDataEx& reflection,
    Material* material,
    CameraResource* cameraResource,
    RHIBuffer* perDrawBuffer,
    LightResource* lightResource
)
{
    std::vector<ResourceBinding> bindings;
    auto appendBufferBinding = [&bindings](const ShaderBufferBindingInfo& buf, RHIBuffer* targetBuffer)
    {
        ResourceBinding binding{};
        binding.type               = buf.resourceType;
        binding.stage              = buf.stages;
        binding.binding            = static_cast<uint8>(buf.binding);
        binding.arrayCount         = 1;
        binding.name               = buf.name;
        binding.nameHash           = buf.nameHash;
        binding.nativeBindingSlots = buf.nativeBindingSlots;
        binding.resource.buffers.push_back(targetBuffer);
        binding.resource.offsets.push_back(0);
        bindings.push_back(std::move(binding));
    };

    auto appendTextureBinding = [&bindings](const ShaderTextureBindingInfo& tex, RHITexture* texture)
    {
        ResourceBinding binding{};
        binding.type               = EResourceType::SampledImage;
        binding.stage              = tex.stages;
        binding.binding            = static_cast<uint8>(tex.binding);
        binding.arrayCount         = 1;
        binding.name               = tex.name;
        binding.nameHash           = tex.nameHash;
        binding.nativeBindingSlots = tex.nativeBindingSlots;
        binding.resource.textures.push_back(texture);
        bindings.push_back(std::move(binding));
    };

    auto appendSamplerBinding = [&bindings](const ShaderSamplerBindingInfo& samp, RHISampler* sampler)
    {
        ResourceBinding binding{};
        binding.type               = EResourceType::Sampler;
        binding.stage              = samp.stages;
        binding.binding            = static_cast<uint8>(samp.binding);
        binding.arrayCount         = 1;
        binding.name               = samp.name;
        binding.nameHash           = samp.nameHash;
        binding.nativeBindingSlots = samp.nativeBindingSlots;
        binding.resource.samplers.push_back(sampler);
        bindings.push_back(std::move(binding));
    };

    auto appendCombinedBinding = [&bindings](const ShaderTextureBindingInfo& tex, RHITexture* texture, RHISampler* sampler)
    {
        ResourceBinding binding{};
        binding.type               = EResourceType::CombinedImageSampler;
        binding.stage              = tex.stages;
        binding.binding            = static_cast<uint8>(tex.binding);
        binding.arrayCount         = 1;
        binding.name               = tex.name;
        binding.nameHash           = tex.nameHash;
        binding.nativeBindingSlots = tex.nativeBindingSlots;
        binding.resource.textures.push_back(texture);
        binding.resource.samplers.push_back(sampler);
        bindings.push_back(std::move(binding));
    };

    // 1. Buffer bindings (perView, perDraw)
    for (const auto& buf : reflection.bufferBindings)
    {
        // Determine which RHI buffer to bind
        RHIBuffer* targetBuffer = nullptr;
        if (buf.name == "perView" || buf.name == "PerView")
        {
            targetBuffer = cameraResource ? cameraResource->perViewBuffer : nullptr;
        }
        else if (buf.name == "perDraw" || buf.name == "PerDraw")
        {
            targetBuffer = perDrawBuffer;
        }
        else if (buf.name == "lightUBO" || buf.name == "LightUBO")
        {
            targetBuffer = lightResource ? lightResource->lightBuffer : nullptr;
        }
        else
        {
            // Per-material or other buffer - skip for now (will be handled later)
            continue;
        }

        appendBufferBinding(buf, targetBuffer);
    }

    // 2. Texture bindings (combined image sampler)
    if (material)
    {
        for (const auto& tex : reflection.textureBindings)
        {
            // Map shader texture name to material texture type
            EMaterialTextureType texType = mapTextureNameToType(tex.name);
            Image* image                 = material->GetTexture(texType);

            if (!image)
            {
                HS_LOG(debug, "[RenderResourceManager] No texture for '%s' (type %d)", tex.name.c_str(), static_cast<int>(texType));
                continue;
            }

            ImageResource* imgRes = GetOrCreateImageResource(image);
            if (!imgRes || !imgRes->isValid)
            {
                HS_LOG(warning, "[RenderResourceManager] Failed to create ImageResource for '%s'", tex.name.c_str());
                continue;
            }
            const ShaderSamplerBindingInfo* matchedSampler = nullptr;
            for (const auto& samp : reflection.samplerBindings)
            {
                if (samp.nameHash == tex.nameHash && samp.stages == tex.stages)
                {
                    matchedSampler = &samp;
                    break;
                }
                if (!matchedSampler && samp.nameHash == tex.nameHash)
                {
                    matchedSampler = &samp;
                }
            }

            if (!matchedSampler && !reflection.samplerBindings.empty())
            {
                matchedSampler = &reflection.samplerBindings.front();
            }

            if (matchedSampler)
            {
                appendTextureBinding(tex, imgRes->texture);
                appendSamplerBinding(*matchedSampler, imgRes->sampler);
            }
            else
            {
                appendCombinedBinding(tex, imgRes->texture, imgRes->sampler);
            }

            HS_LOG(info, "[RenderResourceManager] Bound texture '%s' at binding %u", tex.name.c_str(), tex.binding);
        }
    }

    if (bindings.empty())
    {
        HS_LOG(warning, "[RenderResourceManager] No bindings created from reflection");
    }

    return _rhiContext->CreateResourceLayout(
        "AutoLayout",
        bindings.data(),
        static_cast<uint32>(bindings.size())
    );
}

std::vector<float> RenderResourceManager::buildInterleavedVertexData(
    Mesh* mesh, const ShaderVertexInputLayout& vertexLayout
)
{
    uint32 vertexCount = mesh->GetVertexCount();
    if (vertexCount == 0) return {};

    // Calculate floats per vertex based on stride
    uint32 floatsPerVertex = vertexLayout.stride / sizeof(float);
    if (floatsPerVertex == 0) return {};

    std::vector<float> data(vertexCount * floatsPerVertex, 0.0f);

    const auto& positions  = mesh->GetPosition();
    const auto& normals    = mesh->GetNormal();
    const auto& texcoords0 = mesh->GetTexCoord(0);
    const auto& colors     = mesh->GetColor();
    const auto& tangents   = mesh->GetTangent();

    for (const auto& attr : vertexLayout.attributes)
    {
        uint32 floatOffset = attr.offset / sizeof(float);
        uint32 components  = attr.size / sizeof(float);

        // Match semantic to mesh data
        std::string sem = attr.semantic;
        // Normalize: uppercase comparison
        for (auto& c : sem) c = static_cast<char>(toupper(c));

        const float* srcData = nullptr;
        uint32 srcComponents = 0;

        if (sem.find("POSITION") != std::string::npos)
        {
            srcData       = positions.data();
            srcComponents = 3;
        }
        else if (sem.find("NORMAL") != std::string::npos)
        {
            srcData       = normals.empty() ? nullptr : normals.data();
            srcComponents = 3;
        }
        else if (sem.find("TEXCOORD") != std::string::npos)
        {
            srcData       = texcoords0.empty() ? nullptr : texcoords0.data();
            srcComponents = 2;
        }
        else if (sem.find("COLOR") != std::string::npos)
        {
            srcData       = colors.empty() ? nullptr : colors.data();
            srcComponents = 4;
        }
        else if (sem.find("TANGENT") != std::string::npos)
        {
            srcData       = tangents.empty() ? nullptr : tangents.data();
            srcComponents = 3;
        }

        if (!srcData) continue;

        uint32 copyComponents = (components < srcComponents) ? components : srcComponents;

        for (uint32 v = 0; v < vertexCount; ++v)
        {
            for (uint32 c = 0; c < copyComponents; ++c)
            {
                data[v * floatsPerVertex + floatOffset + c] = srcData[v * srcComponents + c];
            }
        }
    }

    return data;
}

void RenderResourceManager::ReleaseAll()
{
    if (!_rhiContext) return;

    for (auto& [key, res] : _drawResources)
    {
        if (res.resourceSet) _rhiContext->DestroyResourceSet(res.resourceSet);
        if (res.resourceLayout) _rhiContext->DestroyResourceLayout(res.resourceLayout);
    }
    _drawResources.clear();

    for (auto& [mat, res] : _materialResources)
    {
        for (auto& [key, pipeline] : res.pipelineCache)
        {
            if (pipeline) _rhiContext->DestroyGraphicsPipeline(pipeline);
        }
        if (res.resourceSet) _rhiContext->DestroyResourceSet(res.resourceSet);
        if (res.resourceLayout) _rhiContext->DestroyResourceLayout(res.resourceLayout);
        if (res.vertexShader) _rhiContext->DestroyShader(res.vertexShader);
        if (res.fragmentShader) _rhiContext->DestroyShader(res.fragmentShader);
        for (auto* buf : res.materialBuffers)
        {
            if (buf) _rhiContext->DestroyBuffer(buf);
        }
    }
    _materialResources.clear();

    for (auto& [mesh, res] : _meshResources)
    {
        if (res.vertexBuffer) _rhiContext->DestroyBuffer(res.vertexBuffer);
        if (res.indexBuffer) _rhiContext->DestroyBuffer(res.indexBuffer);
    }
    _meshResources.clear();

    for (auto& [camera, res] : _cameraResources)
    {
        if (res.perViewBuffer) _rhiContext->DestroyBuffer(res.perViewBuffer);
    }
    _cameraResources.clear();

    for (auto& [light, res] : _lightResources)
    {
        if (res.lightBuffer) _rhiContext->DestroyBuffer(res.lightBuffer);
    }
    _lightResources.clear();

    for (auto& [model, buffer] : _perDrawBuffers)
    {
        if (buffer) _rhiContext->DestroyBuffer(buffer);
    }
    _perDrawBuffers.clear();
    _perDrawBufferVersions.clear();

    for (auto& [image, res] : _imageResources)
    {
        if (res.texture) _rhiContext->DestroyTexture(res.texture);
        if (res.sampler) _rhiContext->DestroySampler(res.sampler);
    }
    _imageResources.clear();

}

HS_NS_END
