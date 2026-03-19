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

CameraResource* RenderResourceManager::GetOrCreateCameraResource(CameraComponent* cameraComponent)
{
    auto it = _cameraResources.find(cameraComponent);
    if (it != _cameraResources.end() && it->second.isValid)
    {
        auto* resource = &it->second;

        // TODO: UpdateBuffer는 CPU-GPU 동기화 이슈가 있을 수 있으니, 실제로는 더 효율적인 업데이트 전략이 필요할 수 있다 (예: Persistent Mapped Buffer + Ring Buffer)
        // TODO: cameraComponent이 자주 변경되지 않는다면, 매 프레임 업데이트하는 대신 변경된 경우에만 업데이트하도록 최적화할 수 있다 (예: CameraComponent에 dirty flag 추가)
        _rhiContext->UpdateBuffer(resource->perViewBuffer, 0, &resource->perViewData, sizeof(PerView));
    }
    else
    {
        CameraResource resource{};

        PerView perViewZero{};
        resource.perViewBuffer = _rhiContext->CreateBuffer(
            "PerView UBO", &perViewZero, sizeof(PerView),
            EBufferUsage::Uniform, EBufferMemoryOption::Dynamic
        );

        if (!resource.perViewBuffer)
        {
            HS_LOG(error, "[RenderResourceManager] Failed to create PerView buffer");
            return nullptr;
        }

        resource.isValid                  = true;
        _cameraResources[cameraComponent] = std::move(resource);

        HS_LOG(info, "[RenderResourceManager] CameraResource created");
    }
    return &_cameraResources[cameraComponent];
}

void RenderResourceManager::SetActiveCameraResource(CameraResource* resource)
{
    _activeCameraResource = resource;
}

LightResource* RenderResourceManager::GetOrCreateLightResource(LightComponent* light, TransformComponent* transform)
{
    bool created = false;

    auto it = _lightResources.find(light);

    if (it == _lightResources.end())
    {
        LightResource resource{};
        _lightResources[light] = resource;
        created                = true;
    }
    LightResource* resource = &_lightResources[light];

    LightUBO lightBuffer{};
    lightBuffer.position  = glm::vec4(transform->GetWorldPosition(), 0.0f);
    lightBuffer.color     = light->color;
    lightBuffer.intensity = light->intensity;
    lightBuffer.direction = transform->GetForward();
    lightBuffer.type      = static_cast<int>(light->type);

    if (created)
    {
        resource->lightBuffer = _rhiContext->CreateBuffer("Light Buffer", &lightBuffer, sizeof(LightUBO), EBufferUsage::Uniform, EBufferMemoryOption::Dynamic);
    }
    else
    {
        // TODO: UpdateBuffer는 CPU-GPU 동기화 이슈가 있을 수 있으니, 실제로는 더 효율적인 업데이트 전략이 필요할 수 있다 (예: Persistent Mapped Buffer + Ring Buffer)
        // TODO: light이 자주 변경되지 않는다면, 매 프레임 업데이트하는 대신 변경된 경우에만 업데이트하도록 최적화할 수 있다 (예: LightComponent에 dirty flag 추가)
        _rhiContext->UpdateBuffer(resource->lightBuffer, 0, &lightBuffer, sizeof(LightUBO));
    }

    return &_lightResources[light];
}

RHIBuffer* RenderResourceManager::getOrCreatePerDrawBuffer(TransformComponent* transform)
{
    RHIBuffer* buffer = nullptr;

    PerDraw perDraw{
        .modelMatrix        = transform->GetLocalMatrix(),
        .inverseModelMatrix = glm::inverse(transform->GetLocalMatrix())
    };

    auto it = _perDrawBuffers.find(transform);
    if (it == _perDrawBuffers.end())
    {
        buffer = _rhiContext->CreateBuffer(
            "PerDraw UBO", &perDraw, sizeof(PerDraw),
            EBufferUsage::Uniform, EBufferMemoryOption::Dynamic
        );
        _perDrawBuffers[transform] = buffer;
    }
    else
    {
        // TODO: UpdateBuffer는 CPU-GPU 동기화 이슈가 있을 수 있으니, 실제로는 더 효율적인 업데이트 전략이 필요할 수 있다 (예: Persistent Mapped Buffer + Ring Buffer)
        // TODO: transform이 자주 변경되지 않는다면, 매 프레임 업데이트하는 대신 변경된 경우에만 업데이트하도록 최적화할 수 있다 (예: TransformComponent에 dirty flag 추가)
        // 현재는 간단히 매 호출마다 업데이트하도록 구현
        buffer = _perDrawBuffers[transform];
        _rhiContext->UpdateBuffer(buffer, 0, &perDraw, sizeof(PerDraw));
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
        CameraResource* camRes = GetOrCreateCameraResource(&camera);
        if (camRes)
        {
            camRes->perViewData = perView;
            sceneResource.cameraResources.push_back(camRes);
        }
        ++cameraIndex;
    }

    // 2. Lights (TODO: Scene LightComponent → LightResource)
    auto lightView = registry.view<TransformComponent, LightComponent>();
    for (auto [entity, transform, light] : lightView.each())
    {
        LightUBO lightUBO       = {};
        LightResource* lightRes = GetOrCreateLightResource(&light, &transform);

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

        RHIBuffer* perDrawBuffer = getOrCreatePerDrawBuffer(&transform);
        if (!perDrawBuffer) continue;

        // Set active resources for layout creation
        SetActiveCameraResource(
            sceneResource.cameraResources.empty() ? nullptr : sceneResource.cameraResources[0]
        );
        _activePerDrawBuffer = perDrawBuffer;

        MaterialResource* matRes = GetOrCreateMaterialResources(mat);
        if (!matRes) continue;

        MeshResource* meshRes = GetOrCreateMeshResources(mesh, reflection);
        if (!meshRes) continue;

        RenderModel renderModel;
        renderModel.worldMatrix        = transform.worldMatrix;
        renderModel.inverseWorldMatrix = glm::inverse(renderModel.worldMatrix);
        renderModel.material           = mat;
        renderModel.perDrawBuffer      = perDrawBuffer;
        renderModel.meshResource       = meshRes;
        renderModel.materialResource   = matRes;
        sceneResource.renderModels.push_back(renderModel);
    }

    return sceneResource;
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
    MaterialResource* matRes = GetOrCreateMaterialResources(material);
    if (!matRes) return nullptr;

    // Use renderPass pointer as key for pipeline cache
    size_t pipelineKey = PointerHash(renderPass);

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
    cbDesc.attachmentCount = renderPass->info.colorAttachmentCount;
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
    gpInfo.renderPass        = renderPass;
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

    // Create resource layout from reflection (uses active camera/model resources)
    resources.resourceLayout = createResourceLayoutFromReflection(reflection, material);

    if (!resources.resourceLayout)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create resource layout");
        return resources;
    }

    // Create resource set
    resources.resourceSet = _rhiContext->CreateResourceSet("AutoResourceSet", resources.resourceLayout);
    if (resources.resourceSet)
    {
        resources.resourceSet->layouts.push_back(resources.resourceLayout);
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
    const ShaderReflectionDataEx& reflection, Material* material
)
{
    std::vector<ResourceBinding> bindings;

    // 1. Buffer bindings (perView, perDraw)
    for (const auto& buf : reflection.bufferBindings)
    {
        // Determine which RHI buffer to bind
        RHIBuffer* targetBuffer = nullptr;
        if (buf.name == "perView" || buf.name == "PerView")
        {
            targetBuffer = _activeCameraResource ? _activeCameraResource->perViewBuffer : nullptr;
        }
        else if (buf.name == "perDraw" || buf.name == "PerDraw")
        {
            targetBuffer = _activePerDrawBuffer;
        }
        else if (buf.name == "lightUBO" || buf.name == "LightUBO")
        {
            targetBuffer = _lightResources.begin()->second.lightBuffer; // TODO: 고쳐야함!! Additional Lights를 지원해야 됨
        }
        else
        {
            // Per-material or other buffer - skip for now (will be handled later)
            continue;
        }

        if (!targetBuffer) continue;

#ifdef __APPLE__
        // Metal: separate bindings per stage
        if ((buf.stages & EShaderStage::Vertex) != EShaderStage::None)
        {
            ResourceBinding binding{};
            binding.type       = buf.resourceType;
            binding.stage      = EShaderStage::Vertex;
            binding.binding    = static_cast<uint8>(buf.binding);
            binding.arrayCount = 1;
            binding.resource.buffers.push_back(targetBuffer);
            binding.resource.offsets.push_back(0);
            bindings.push_back(std::move(binding));
        }
        if ((buf.stages & EShaderStage::Fragment) != EShaderStage::None)
        {
            ResourceBinding binding{};
            binding.type       = buf.resourceType;
            binding.stage      = EShaderStage::Fragment;
            binding.binding    = static_cast<uint8>(buf.binding);
            binding.arrayCount = 1;
            binding.resource.buffers.push_back(targetBuffer);
            binding.resource.offsets.push_back(0);
            bindings.push_back(std::move(binding));
        }
#elif __WINDOWS__
        // Vulkan: combined stage visibility
        ResourceBinding binding{};
        binding.type       = buf.resourceType;
        binding.stage      = buf.stages;
        binding.binding    = static_cast<uint8>(buf.binding);
        binding.arrayCount = 1;
        binding.resource.buffers.push_back(targetBuffer);
        binding.resource.offsets.push_back(0);
        bindings.push_back(std::move(binding));
#endif
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

#ifdef __APPLE__
            // Metal: separate texture and sampler bindings per stage
            if ((tex.stages & EShaderStage::Fragment) != EShaderStage::None)
            {
                ResourceBinding texBinding{};
                texBinding.type       = EResourceType::SampledImage;
                texBinding.stage      = EShaderStage::Fragment;
                texBinding.binding    = static_cast<uint8>(tex.binding);
                texBinding.arrayCount = 1;
                texBinding.resource.textures.push_back(imgRes->texture);
                bindings.push_back(std::move(texBinding));

                // Find matching sampler binding
                for (const auto& samp : reflection.samplerBindings)
                {
                    if ((samp.stages & EShaderStage::Fragment) != EShaderStage::None)
                    {
                        ResourceBinding sampBinding{};
                        sampBinding.type       = EResourceType::Sampler;
                        sampBinding.stage      = EShaderStage::Fragment;
                        sampBinding.binding    = static_cast<uint8>(samp.binding);
                        sampBinding.arrayCount = 1;
                        sampBinding.resource.samplers.push_back(imgRes->sampler);
                        bindings.push_back(std::move(sampBinding));
                        break; // Use first matching sampler
                    }
                }
            }
#elif __WINDOWS__
            // Vulkan: combined image sampler
            ResourceBinding binding{};
            binding.type       = EResourceType::CombinedImageSampler;
            binding.stage      = tex.stages;
            binding.binding    = static_cast<uint8>(tex.binding);
            binding.arrayCount = 1;
            binding.resource.textures.push_back(imgRes->texture);
            binding.resource.samplers.push_back(imgRes->sampler);
            bindings.push_back(std::move(binding));

            HS_LOG(info, "[RenderResourceManager] Bound texture '%s' at binding %u", tex.name.c_str(), tex.binding);
#endif
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

    for (auto& [model, buffer] : _perDrawBuffers)
    {
        if (buffer) _rhiContext->DestroyBuffer(buffer);
    }
    _perDrawBuffers.clear();

    for (auto& [image, res] : _imageResources)
    {
        if (res.texture) _rhiContext->DestroyTexture(res.texture);
        if (res.sampler) _rhiContext->DestroySampler(res.sampler);
    }
    _imageResources.clear();

    _activeCameraResource = nullptr;
    _activePerDrawBuffer  = nullptr;
}

HS_NS_END
