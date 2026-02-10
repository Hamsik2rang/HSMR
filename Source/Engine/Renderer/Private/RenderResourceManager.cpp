#include "Renderer/RenderResourceManager.h"

#include "Core/Log.h"
#include "Core/Hash.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"

#include "Resource/Material.h"
#include "Resource/Mesh.h"
#include "Resource/Shader.h"

HS_NS_BEGIN

RenderResourceManager::RenderResourceManager(RHIContext* rhiContext)
    : _rhiContext(rhiContext)
{
}

RenderResourceManager::~RenderResourceManager()
{
    ReleaseAll();
}

CameraResource* RenderResourceManager::GetOrCreateCameraResource(Camera* camera)
{
    auto it = _cameraResources.find(camera);
    if (it != _cameraResources.end() && it->second.isValid)
    {
        return &it->second;
    }

    CameraResource resource;

    PerView perViewZero{};
    resource.perViewBuffer = _rhiContext->CreateBuffer(
        "PerView UBO", &perViewZero, sizeof(PerView),
        EBufferUsage::UNIFORM, EBufferMemoryOption::DYNAMIC);

    if (!resource.perViewBuffer)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create PerView buffer");
        return nullptr;
    }

    resource.isValid = true;
    _cameraResources[camera] = std::move(resource);

    HS_LOG(info, "[RenderResourceManager] CameraResource created");
    return &_cameraResources[camera];
}

void RenderResourceManager::SetActiveCameraResource(CameraResource* resource)
{
    _activeCameraResource = resource;
}

ModelResource* RenderResourceManager::GetOrCreateModelResource(Model* model)
{
    auto it = _modelResources.find(model);
    if (it != _modelResources.end() && it->second.isValid)
    {
        return &it->second;
    }

    ModelResource resource;

    PerDraw perDrawZero{};
    resource.perDrawBuffer = _rhiContext->CreateBuffer(
        "PerDraw UBO", &perDrawZero, sizeof(PerDraw),
        EBufferUsage::UNIFORM, EBufferMemoryOption::DYNAMIC);

    if (!resource.perDrawBuffer)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create PerDraw buffer");
        return nullptr;
    }

    resource.isValid = true;
    _modelResources[model] = std::move(resource);

    HS_LOG(info, "[RenderResourceManager] ModelResource created");
    return &_modelResources[model];
}

void RenderResourceManager::SetActiveModelResource(ModelResource* resource)
{
    _activeModelResource = resource;
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
    viLayout.binding = 0;
    viLayout.stride = reflection.vertexInput.stride;
    viLayout.stepRate = 1;
    viLayout.useInstancing = false;
    viDesc.layouts.push_back(viLayout);

    for (const auto& attr : reflection.vertexInput.attributes)
    {
        VertexInputAttributeDescriptor viAttr{};
        viAttr.location = attr.location;
        viAttr.binding = 0;
        viAttr.format = attr.format;
        viAttr.offset = attr.offset;
        viDesc.attributes.push_back(viAttr);
    }

    // Standard pipeline settings
    DepthStencilStateDescriptor dsDesc{};
    dsDesc.depthTestEnable = true;
    dsDesc.depthWriteEnable = true;
    dsDesc.depthCompareOp = ECompareOp::LESS;

    ColorBlendStateDescriptor cbDesc{};
    cbDesc.attachmentCount = renderPass->info.colorAttachmentCount;
    cbDesc.attachments.resize(cbDesc.attachmentCount);
    for (size_t i = 0; i < cbDesc.attachmentCount; ++i)
    {
        cbDesc.attachments[i].blendEnable = false;
    }

    RasterizerStateDescriptor rsDesc{};
    rsDesc.cullMode = material->IsTwoSided() ? ECullMode::NONE : ECullMode::BACK;
    rsDesc.frontFace = EFrontFace::CCW;
    rsDesc.polygonMode = EPolygonMode::FILL;
    rsDesc.depthClampEnable = false;
    rsDesc.rasterizerDiscardEnable = false;
    rsDesc.depthBiasEnable = false;

    ShaderProgramDescriptor spDesc{};
    spDesc.stages.resize(2);
    spDesc.stages[0] = matRes->vertexShader;
    spDesc.stages[1] = matRes->fragmentShader;

    InputAssemblyStateDescriptor iaDesc{};
    iaDesc.primitiveTopology = EPrimitiveTopology::TRIANGLE_LIST;

    GraphicsPipelineInfo gpInfo{};
    gpInfo.shaderDesc = spDesc;
    gpInfo.inputAssemblyDesc = iaDesc;
    gpInfo.vertexInputDesc = viDesc;
    gpInfo.rasterizerDesc = rsDesc;
    gpInfo.depthStencilDesc = dsDesc;
    gpInfo.colorBlendDesc = cbDesc;
    gpInfo.renderPass = renderPass;
    gpInfo.resourceLayout = matRes->resourceLayout;

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
    const auto& indices = mesh->GetIndices();
    if (positions.empty() || indices.empty()) return nullptr;

    MeshResource resources;

    // Build interleaved vertex data based on reflection
    std::vector<float> interleavedData = buildInterleavedVertexData(mesh, reflection.vertexInput);

    if (interleavedData.empty()) return nullptr;

    resources.vertexBuffer = _rhiContext->CreateBuffer(
        "Mesh VB", interleavedData.data(),
        interleavedData.size() * sizeof(float),
        EBufferUsage::VERTEX, EBufferMemoryOption::MAPPED);

    resources.indexBuffer = _rhiContext->CreateBuffer(
        "Mesh IB", indices.data(),
        indices.size() * sizeof(uint32),
        EBufferUsage::INDEX, EBufferMemoryOption::MAPPED);

    resources.indexCount = static_cast<uint32>(indices.size());
    resources.isValid = true;

    _meshResources[mesh] = std::move(resources);
    return &_meshResources[mesh];
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

    // Create RHI shaders from bytecode
    const auto* vsBytecode = shader->GetBytecode(EShaderStage::VERTEX);
    const auto* fsBytecode = shader->GetBytecode(EShaderStage::FRAGMENT);

    if (!vsBytecode || !fsBytecode)
    {
        HS_LOG(error, "[RenderResourceManager] Missing vertex or fragment bytecode");
        return resources;
    }

    ShaderInfo vsInfo{};
    vsInfo.stage = EShaderStage::VERTEX;
    vsInfo.entryName = shader->GetEntryPoint(EShaderStage::VERTEX);
    resources.vertexShader = _rhiContext->CreateShader(
        "VS", vsInfo,
        reinterpret_cast<const char*>(vsBytecode->data()),
        vsBytecode->size());

    ShaderInfo fsInfo{};
    fsInfo.stage = EShaderStage::FRAGMENT;
    fsInfo.entryName = shader->GetEntryPoint(EShaderStage::FRAGMENT);
    resources.fragmentShader = _rhiContext->CreateShader(
        "FS", fsInfo,
        reinterpret_cast<const char*>(fsBytecode->data()),
        fsBytecode->size());

    if (!resources.vertexShader || !resources.fragmentShader)
    {
        HS_LOG(error, "[RenderResourceManager] Failed to create RHI shaders");
        return resources;
    }

    // Create resource layout from reflection (uses active camera/model resources)
    resources.resourceLayout = createResourceLayoutFromReflection(reflection);

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
    HS_LOG(info, "[RenderResourceManager] Material resources created for shader '%s'",
           shader->GetShaderName().c_str());
    return resources;
}

RHIResourceLayout* RenderResourceManager::createResourceLayoutFromReflection(
    const ShaderReflectionDataEx& reflection)
{
    std::vector<ResourceBinding> bindings;

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
            targetBuffer = _activeModelResource ? _activeModelResource->perDrawBuffer : nullptr;
        }
        else
        {
            // Per-material or other buffer - skip for now (will be handled later)
            continue;
        }

        if (!targetBuffer) continue;

#ifdef __APPLE__
        // Metal: separate bindings per stage
        if ((buf.stages & EShaderStage::VERTEX) != EShaderStage::NONE)
        {
            ResourceBinding binding{};
            binding.type = buf.resourceType;
            binding.stage = EShaderStage::VERTEX;
            binding.binding = static_cast<uint8>(buf.binding);
            binding.arrayCount = 1;
            binding.resource.buffers.push_back(targetBuffer);
            binding.resource.offsets.push_back(0);
            bindings.push_back(std::move(binding));
        }
        if ((buf.stages & EShaderStage::FRAGMENT) != EShaderStage::NONE)
        {
            ResourceBinding binding{};
            binding.type = buf.resourceType;
            binding.stage = EShaderStage::FRAGMENT;
            binding.binding = static_cast<uint8>(buf.binding);
            binding.arrayCount = 1;
            binding.resource.buffers.push_back(targetBuffer);
            binding.resource.offsets.push_back(0);
            bindings.push_back(std::move(binding));
        }
#elif __WINDOWS__
        // Vulkan: combined stage visibility
        ResourceBinding binding{};
        binding.type = buf.resourceType;
        binding.stage = buf.stages;
        binding.binding = static_cast<uint8>(buf.binding);
        binding.arrayCount = 1;
        binding.resource.buffers.push_back(targetBuffer);
        binding.resource.offsets.push_back(0);
        bindings.push_back(std::move(binding));
#endif
    }

    if (bindings.empty())
    {
        HS_LOG(warning, "[RenderResourceManager] No bindings created from reflection");
    }

    return _rhiContext->CreateResourceLayout(
        "AutoLayout",
        bindings.data(),
        static_cast<uint32>(bindings.size()));
}

std::vector<float> RenderResourceManager::buildInterleavedVertexData(
    Mesh* mesh, const ShaderVertexInputLayout& vertexLayout)
{
    uint32 vertexCount = mesh->GetVertexCount();
    if (vertexCount == 0) return {};

    // Calculate floats per vertex based on stride
    uint32 floatsPerVertex = vertexLayout.stride / sizeof(float);
    if (floatsPerVertex == 0) return {};

    std::vector<float> data(vertexCount * floatsPerVertex, 0.0f);

    const auto& positions = mesh->GetPosition();
    const auto& normals = mesh->GetNormal();
    const auto& texcoords0 = mesh->GetTexCoord(0);
    const auto& colors = mesh->GetColor();
    const auto& tangents = mesh->GetTangent();

    for (const auto& attr : vertexLayout.attributes)
    {
        uint32 floatOffset = attr.offset / sizeof(float);
        uint32 components = attr.size / sizeof(float);

        // Match semantic to mesh data
        std::string sem = attr.semantic;
        // Normalize: uppercase comparison
        for (auto& c : sem) c = static_cast<char>(toupper(c));

        const float* srcData = nullptr;
        uint32 srcComponents = 0;

        if (sem.find("POSITION") != std::string::npos)
        {
            srcData = positions.data();
            srcComponents = 3;
        }
        else if (sem.find("NORMAL") != std::string::npos)
        {
            srcData = normals.empty() ? nullptr : normals.data();
            srcComponents = 3;
        }
        else if (sem.find("TEXCOORD") != std::string::npos)
        {
            srcData = texcoords0.empty() ? nullptr : texcoords0.data();
            srcComponents = 2;
        }
        else if (sem.find("COLOR") != std::string::npos)
        {
            srcData = colors.empty() ? nullptr : colors.data();
            srcComponents = 4;
        }
        else if (sem.find("TANGENT") != std::string::npos)
        {
            srcData = tangents.empty() ? nullptr : tangents.data();
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

    for (auto& [model, res] : _modelResources)
    {
        if (res.perDrawBuffer) _rhiContext->DestroyBuffer(res.perDrawBuffer);
    }
    _modelResources.clear();

    _activeCameraResource = nullptr;
    _activeModelResource = nullptr;
}

HS_NS_END
