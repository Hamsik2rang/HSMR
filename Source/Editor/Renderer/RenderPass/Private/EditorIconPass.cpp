#include "Editor/Renderer/RenderPass/EditorIconPass.h"

#include "Core/Hash.h"
#include "Core/Log.h"
#include "Core/SystemContext.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"

#include "Renderer/RenderResourceManager.h"
#include "Renderer/ShaderLibrary.h"

#include "Resource/ObjectManager.h"
#include "Resource/Shader.h"

#include <algorithm>
#include <cstddef>

HS_NS_BEGIN

namespace
{
constexpr float s_fadeStart = 5.0f;
constexpr float s_fadeEnd = 30.0f;
constexpr float s_minAlpha = 0.1f;
constexpr float s_targetPixelSize = 128.0f;
constexpr float s_perspectiveProjectionSentinel = 0.0001f;

const glm::vec3 s_cameraIconTint{0.62f, 0.84f, 1.0f};
const glm::vec3 s_lightIconTint{1.0f, 0.92f, 0.56f};

size_t buildPipelineKey(const PipelineRenderTargetLayout& renderTargetLayout)
{
    return std::hash<PipelineRenderTargetLayout>{}(renderTargetLayout);
}

float computeDistanceFade(const glm::vec3& cameraPosition, const glm::vec3& worldPosition)
{
    float distance = glm::distance(cameraPosition, worldPosition);
    float fade = 1.0f - glm::clamp((distance - s_fadeStart) / (s_fadeEnd - s_fadeStart), 0.0f, 1.0f);
    return glm::mix(s_minAlpha, 1.0f, fade);
}

float computeWorldIconSize(const RenderViewSnapshot& viewSnapshot, const glm::vec3& worldPosition)
{
    const glm::mat4& viewMatrix = viewSnapshot.perView.viewMatrix;
    const glm::mat4& projectionMatrix = viewSnapshot.perView.projectionMatrix;
    const glm::vec2 resolution = glm::vec2(viewSnapshot.perView.resolution);

    const float projYY = projectionMatrix[1][1];
    const bool isPerspective = glm::abs(projectionMatrix[3][3]) < s_perspectiveProjectionSentinel;
    const float resolutionY = glm::max(resolution.y, 1.0f);

    if (isPerspective)
    {
        const glm::vec3 viewPosition = glm::vec3(viewMatrix * glm::vec4(worldPosition, 1.0f));
        const float depth = glm::max(viewPosition.z, 0.001f);
        return (2.0f * depth * s_targetPixelSize) / glm::max(projYY * resolutionY, 1.0f);
    }

    return (2.0f * s_targetPixelSize) / glm::max(projYY * resolutionY, 1.0f);
}
}

EditorIconPass::~EditorIconPass()
{
    Shutdown();
}

bool EditorIconPass::Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext)
{
    _rhiContext = rhiContext;

    Shader* shader = shaderLibrary->GetOrCompile("EditorIcon", EShaderStage::Vertex | EShaderStage::Fragment);
    if (!shader || !shader->IsCompiledEx())
    {
        HS_LOG(error, "[EditorIconPass] Failed to compile EditorIcon.slang");
        return false;
    }

    const auto* vsBytecode = shader->GetByteCode(EShaderStage::Vertex);
    const auto* fsBytecode = shader->GetByteCode(EShaderStage::Fragment);
    if (!vsBytecode || !fsBytecode)
    {
        HS_LOG(error, "[EditorIconPass] Missing EditorIcon shader bytecode");
        return false;
    }

    ShaderInfo vsInfo{};
    vsInfo.stage = EShaderStage::Vertex;
    vsInfo.entryName = shader->GetEntryPoint(EShaderStage::Vertex);
    _vertexShader = _rhiContext->CreateShader(
        "EditorIconVS", vsInfo,
        reinterpret_cast<const char*>(vsBytecode->data()),
        vsBytecode->size());

    ShaderInfo fsInfo{};
    fsInfo.stage = EShaderStage::Fragment;
    fsInfo.entryName = shader->GetEntryPoint(EShaderStage::Fragment);
    _fragmentShader = _rhiContext->CreateShader(
        "EditorIconFS", fsInfo,
        reinterpret_cast<const char*>(fsBytecode->data()),
        fsBytecode->size());

    if (!_vertexShader || !_fragmentShader)
    {
        HS_LOG(error, "[EditorIconPass] Failed to create RHI shaders");
        return false;
    }

    const IconVertex quadVertices[] = {
        {{-0.5f, -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f}, {0.0f, 1.0f}},
        {{ 0.5f,  0.5f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f}, {0.0f, 0.0f}},
    };

    _vertexBuffer = _rhiContext->CreateBuffer(
        "EditorIconQuadBuffer",
        quadVertices,
        sizeof(quadVertices),
        EBufferUsage::Vertex,
        EBufferMemoryOption::Static);
    _vertexCount = static_cast<uint32>(std::size(quadVertices));

    if (!_vertexBuffer)
    {
        HS_LOG(error, "[EditorIconPass] Failed to create quad vertex buffer");
        return false;
    }

    _isInitialized = true;
    return true;
}

void EditorIconPass::Shutdown()
{
    if (!_rhiContext)
    {
        return;
    }

    _rhiContext->WaitForIdle();

    resetPipelines();

    if (_instanceBuffer)
    {
        _rhiContext->DestroyBuffer(_instanceBuffer);
        _instanceBuffer = nullptr;
    }

    if (_vertexBuffer)
    {
        _rhiContext->DestroyBuffer(_vertexBuffer);
        _vertexBuffer = nullptr;
    }

    if (_resourceSet)
    {
        _rhiContext->DestroyResourceSet(_resourceSet);
        _resourceSet = nullptr;
    }

    if (_resourceLayout)
    {
        _rhiContext->DestroyResourceLayout(_resourceLayout);
        _resourceLayout = nullptr;
    }

    if (_fragmentShader)
    {
        _rhiContext->DestroyShader(_fragmentShader);
        _fragmentShader = nullptr;
    }

    if (_vertexShader)
    {
        _rhiContext->DestroyShader(_vertexShader);
        _vertexShader = nullptr;
    }

    _cameraIconImage.reset();
    _lightIconImage.reset();
    _cameraIconResource = nullptr;
    _lightIconResource = nullptr;
    _instances.clear();
    _instanceCapacity = 0;
    _instanceCount = 0;
    _vertexCount = 0;
    _perViewBuffer = nullptr;
    _isInitialized = false;
    _rhiContext = nullptr;
}

bool EditorIconPass::Prepare(RenderResourceManager& resourceManager,
                              const RenderSceneSnapshot& snapshot,
                              const RenderViewSnapshot& viewSnapshot)
{
    if (!_isInitialized)
    {
        return false;
    }

    _instances.clear();
    _instanceCount = 0;

    if (!ensureIconResources(resourceManager))
    {
        return false;
    }

    for (const DebugCameraSnapshot& camera : snapshot.debugCameras)
    {
        addCameraIcon(camera, viewSnapshot);
    }

    for (const DebugLightSnapshot& light : snapshot.debugLights)
    {
        if (light.isEnabled)
        {
            addLightIcon(light, viewSnapshot);
        }
    }

    _instanceCount = static_cast<uint32>(_instances.size());
    if (_instanceCount == 0)
    {
        return true;
    }

    if (_instanceCount > _instanceCapacity)
    {
        if (_instanceBuffer)
        {
            _rhiContext->DestroyBuffer(_instanceBuffer);
            _instanceBuffer = nullptr;
        }

        _instanceCapacity = std::max<uint32>(_instanceCount, _instanceCapacity == 0 ? 32 : _instanceCapacity * 2);
        _instanceBuffer = _rhiContext->CreateBuffer(
            "EditorIconInstanceBuffer",
            nullptr,
            sizeof(IconInstance) * _instanceCapacity,
            EBufferUsage::Vertex,
            EBufferMemoryOption::Dynamic);
    }

    if (_instanceBuffer)
    {
        _rhiContext->UpdateBuffer(
            _instanceBuffer,
            0,
            _instances.data(),
            sizeof(IconInstance) * _instanceCount);
    }

    return _instanceBuffer != nullptr;
}

RHIGraphicsPipeline* EditorIconPass::GetOrCreatePipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                                          RHIBuffer* perViewBuffer)
{
    if (!_isInitialized || !perViewBuffer || !_cameraIconResource || !_lightIconResource)
    {
        return nullptr;
    }

    if (perViewBuffer != _perViewBuffer)
    {
        rebuildResourceBindings(perViewBuffer);
        resetPipelines();
    }

    if (!_resourceLayout || !_resourceSet)
    {
        return nullptr;
    }

    size_t pipelineKey = buildPipelineKey(renderTargetLayout);
    auto it = _pipelineCache.find(pipelineKey);
    if (it != _pipelineCache.end())
    {
        return it->second;
    }

    VertexInputStateDescriptor viDesc{};

    VertexInputLayoutDescriptor quadLayout{};
    quadLayout.binding = 0;
    quadLayout.stride = sizeof(IconVertex);
    quadLayout.stepRate = 1;
    quadLayout.useInstancing = false;
    viDesc.layouts.push_back(quadLayout);

    VertexInputLayoutDescriptor instanceLayout{};
    instanceLayout.binding = 1;
    instanceLayout.stride = sizeof(IconInstance);
    instanceLayout.stepRate = 1;
    instanceLayout.useInstancing = true;
    viDesc.layouts.push_back(instanceLayout);

    VertexInputAttributeDescriptor localPosAttr{};
    localPosAttr.location = 0;
    localPosAttr.binding = 0;
    localPosAttr.format = EVertexFormat::Float2;
    localPosAttr.offset = offsetof(IconVertex, localPosition);
    viDesc.attributes.push_back(localPosAttr);

    VertexInputAttributeDescriptor uvAttr{};
    uvAttr.location = 1;
    uvAttr.binding = 0;
    uvAttr.format = EVertexFormat::Float2;
    uvAttr.offset = offsetof(IconVertex, uv);
    viDesc.attributes.push_back(uvAttr);

    VertexInputAttributeDescriptor worldAndTypeAttr{};
    worldAndTypeAttr.location = 2;
    worldAndTypeAttr.binding = 1;
    worldAndTypeAttr.format = EVertexFormat::Float4;
    worldAndTypeAttr.offset = offsetof(IconInstance, worldPositionAndSize);
    viDesc.attributes.push_back(worldAndTypeAttr);

    VertexInputAttributeDescriptor tintAttr{};
    tintAttr.location = 3;
    tintAttr.binding = 1;
    tintAttr.format = EVertexFormat::Float4;
    tintAttr.offset = offsetof(IconInstance, tintAndAlpha);
    viDesc.attributes.push_back(tintAttr);

    VertexInputAttributeDescriptor iconMetaAttr{};
    iconMetaAttr.location = 4;
    iconMetaAttr.binding = 1;
    iconMetaAttr.format = EVertexFormat::Float4;
    iconMetaAttr.offset = offsetof(IconInstance, iconMeta);
    viDesc.attributes.push_back(iconMetaAttr);

    InputAssemblyStateDescriptor iaDesc{};
    iaDesc.primitiveTopology = EPrimitiveTopology::TriangleList;
    iaDesc.isRestartEnable = false;

    RasterizerStateDescriptor rsDesc{};
    rsDesc.cullMode = ECullMode::None;
    rsDesc.frontFace = EFrontFace::CounterClockwise;
    rsDesc.polygonMode = EPolygonMode::Fill;
    rsDesc.depthClampEnable = false;
    rsDesc.rasterizerDiscardEnable = false;
    rsDesc.depthBiasEnable = false;
    rsDesc.lineWidth = 1.0f;

    DepthStencilStateDescriptor dsDesc{};
    dsDesc.depthTestEnable = false;
    dsDesc.depthWriteEnable = false;
    dsDesc.depthCompareOp = ECompareOp::Always;
    dsDesc.depthBoundTestEnable = false;
    dsDesc.stencilTestEnable = false;

    ColorBlendStateDescriptor cbDesc{};
    cbDesc.logicOpEnable = false;
    cbDesc.attachmentCount = renderTargetLayout.colorAttachmentCount;
    cbDesc.attachments.resize(cbDesc.attachmentCount);
    for (ColorBlendAttachmentDescriptor& attachment : cbDesc.attachments)
    {
        attachment.blendEnable = true;
        attachment.srcColorFactor = EBlendFactor::SrcAlpha;
        attachment.dstColorFactor = EBlendFactor::OneMinusSrcAlpha;
        attachment.colorBlendOp = EBlendOp::Add;
        attachment.srcAlphaFactor = EBlendFactor::One;
        attachment.dstAlphaFactor = EBlendFactor::OneMinusSrcAlpha;
        attachment.alphaBlendOp = EBlendOp::Add;
    }

    ShaderProgramDescriptor shaderDesc{};
    shaderDesc.stages.push_back(_vertexShader);
    shaderDesc.stages.push_back(_fragmentShader);

    GraphicsPipelineInfo gpInfo{};
    gpInfo.shaderDesc = shaderDesc;
    gpInfo.inputAssemblyDesc = iaDesc;
    gpInfo.vertexInputDesc = viDesc;
    gpInfo.rasterizerDesc = rsDesc;
    gpInfo.depthStencilDesc = dsDesc;
    gpInfo.colorBlendDesc = cbDesc;
    gpInfo.renderTargetLayout = renderTargetLayout;
    gpInfo.resourceLayout = _resourceLayout;

    RHIGraphicsPipeline* pipeline = _rhiContext->CreateGraphicsPipeline("EditorIconPipeline", gpInfo);
    if (!pipeline)
    {
        HS_LOG(error, "[EditorIconPass] Failed to create graphics pipeline");
        return nullptr;
    }

    _pipelineCache[pipelineKey] = pipeline;
    return pipeline;
}

bool EditorIconPass::ensureIconResources(RenderResourceManager& resourceManager)
{
    if (!_cameraIconImage || !_lightIconImage)
    {
        const SystemContext* sysContext = SystemContext::Get();
        if (!sysContext || sysContext->assetDirectory.empty())
        {
            HS_LOG(error, "[EditorIconPass] Missing asset directory");
            return false;
        }

        if (!_cameraIconImage)
        {
            std::string cameraPath = sysContext->assetDirectory + "Editor/Icons/SceneCameraIcon.png";
            _cameraIconImage = ObjectManager::LoadImageFromFile(cameraPath, true);
            if (!_cameraIconImage)
            {
                HS_LOG(error, "[EditorIconPass] Failed to load camera icon: %s", cameraPath.c_str());
                return false;
            }
        }

        if (!_lightIconImage)
        {
            std::string lightPath = sysContext->assetDirectory + "Editor/Icons/SceneLightIcon.png";
            _lightIconImage = ObjectManager::LoadImageFromFile(lightPath, true);
            if (!_lightIconImage)
            {
                HS_LOG(error, "[EditorIconPass] Failed to load light icon: %s", lightPath.c_str());
                return false;
            }
        }
    }

    _cameraIconResource = resourceManager.GetOrCreateImageResource(_cameraIconImage.get());
    _lightIconResource = resourceManager.GetOrCreateImageResource(_lightIconImage.get());

    return _cameraIconResource && _cameraIconResource->isValid &&
           _lightIconResource && _lightIconResource->isValid;
}

void EditorIconPass::rebuildResourceBindings(RHIBuffer* perViewBuffer)
{
    if (_resourceSet)
    {
        _rhiContext->DestroyResourceSet(_resourceSet);
        _resourceSet = nullptr;
    }

    if (_resourceLayout)
    {
        _rhiContext->DestroyResourceLayout(_resourceLayout);
        _resourceLayout = nullptr;
    }

    std::vector<ResourceBinding> bindings;

    ResourceBinding perViewBinding{};
    perViewBinding.type = EResourceType::UniformBuffer;
    perViewBinding.stage = EShaderStage::Vertex;
    perViewBinding.binding = 0;
    perViewBinding.arrayCount = 1;
    perViewBinding.name = "perView";
    perViewBinding.resource.buffers.push_back(perViewBuffer);
    perViewBinding.resource.offsets.push_back(0);
    bindings.push_back(std::move(perViewBinding));

    ResourceBinding cameraIconBinding{};
    cameraIconBinding.type = EResourceType::CombinedImageSampler;
    cameraIconBinding.stage = EShaderStage::Fragment;
    cameraIconBinding.binding = 1;
    cameraIconBinding.nativeBindingSlots.fragmentBinding = 0;  // Metal: texture/sampler slot 0
    cameraIconBinding.arrayCount = 1;
    cameraIconBinding.name = "cameraIconSampler";
    cameraIconBinding.resource.textures.push_back(_cameraIconResource->texture);
    cameraIconBinding.resource.samplers.push_back(_cameraIconResource->sampler);
    bindings.push_back(std::move(cameraIconBinding));

    ResourceBinding lightIconBinding{};
    lightIconBinding.type = EResourceType::CombinedImageSampler;
    lightIconBinding.stage = EShaderStage::Fragment;
    lightIconBinding.binding = 2;
    lightIconBinding.nativeBindingSlots.fragmentBinding = 1;  // Metal: texture/sampler slot 1
    lightIconBinding.arrayCount = 1;
    lightIconBinding.name = "lightIconSampler";
    lightIconBinding.resource.textures.push_back(_lightIconResource->texture);
    lightIconBinding.resource.samplers.push_back(_lightIconResource->sampler);
    bindings.push_back(std::move(lightIconBinding));

    _resourceLayout = _rhiContext->CreateResourceLayout("EditorIconLayout", bindings.data(), static_cast<uint32>(bindings.size()));
    if (!_resourceLayout)
    {
        HS_LOG(error, "[EditorIconPass] Failed to create resource layout");
        return;
    }

    _resourceSet = _rhiContext->CreateResourceSet("EditorIconSet", _resourceLayout);
    if (!_resourceSet)
    {
        HS_LOG(error, "[EditorIconPass] Failed to create resource set");
        return;
    }

    _perViewBuffer = perViewBuffer;
}

void EditorIconPass::resetPipelines()
{
    for (auto& [key, pipeline] : _pipelineCache)
    {
        if (pipeline)
        {
            _rhiContext->DestroyGraphicsPipeline(pipeline);
        }
    }
    _pipelineCache.clear();
}

void EditorIconPass::addCameraIcon(const DebugCameraSnapshot& camera, const RenderViewSnapshot& viewSnapshot)
{
    addIconInstance(
        glm::vec3(camera.worldMatrix[3]),
        viewSnapshot,
        EIconType::Camera,
        s_cameraIconTint,
        computeDistanceFade(glm::vec3(viewSnapshot.perView.cameraPositionTime), glm::vec3(camera.worldMatrix[3])));
}

void EditorIconPass::addLightIcon(const DebugLightSnapshot& light, const RenderViewSnapshot& viewSnapshot)
{
    addIconInstance(
        glm::vec3(light.worldMatrix[3]),
        viewSnapshot,
        EIconType::Light,
        s_lightIconTint,
        computeDistanceFade(glm::vec3(viewSnapshot.perView.cameraPositionTime), glm::vec3(light.worldMatrix[3])));
}

void EditorIconPass::addIconInstance(const glm::vec3& worldPosition,
                                      const RenderViewSnapshot& viewSnapshot,
                                      EIconType iconType,
                                      const glm::vec3& tint,
                                      float alpha)
{
    IconInstance instance{};
    instance.worldPositionAndSize = glm::vec4(worldPosition, computeWorldIconSize(viewSnapshot, worldPosition));
    instance.tintAndAlpha = glm::vec4(tint, glm::clamp(alpha, s_minAlpha, 1.0f));
    instance.iconMeta = glm::vec4(static_cast<float>(iconType == EIconType::Light ? 1 : 0), 0.0f, 0.0f, 0.0f);
    _instances.push_back(instance);
}

HS_NS_END
