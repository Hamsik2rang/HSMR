#include "Renderer/RenderPass/AtmospherePass.h"

#include "Core/Hash.h"
#include "Core/Log.h"

#include "RHI/CommandHandle.h"
#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"

#include "Renderer/ShaderLibrary.h"
#include "Resource/Shader.h"

#include <algorithm>
#include <cmath>

HS_NS_BEGIN

namespace
{
constexpr uint32 TransmittanceWidth = 256;
constexpr uint32 TransmittanceHeight = 64;
constexpr uint32 IrradianceWidth = 64;
constexpr uint32 IrradianceHeight = 16;
constexpr uint32 ScatteringWidth = 256;
constexpr uint32 ScatteringHeight = 128;
constexpr uint32 ScatteringDepth = 32;
constexpr uint32 Rgba16fBytesPerPixel = 8;

bool nearEqual(float a, float b)
{
    return std::abs(a - b) <= 0.0001f;
}

const ShaderBufferBindingInfo* findBufferBinding(const ShaderReflectionDataEx& reflection, const char* name)
{
    for (const ShaderBufferBindingInfo& binding : reflection.bufferBindings)
    {
        if (binding.name == name)
        {
            return &binding;
        }
    }
    return nullptr;
}

const ShaderTextureBindingInfo* findTextureBinding(const ShaderReflectionDataEx& reflection, const char* name)
{
    for (const ShaderTextureBindingInfo& binding : reflection.textureBindings)
    {
        if (binding.name == name)
        {
            return &binding;
        }
    }
    return nullptr;
}

const ShaderSamplerBindingInfo* findSamplerBinding(const ShaderReflectionDataEx& reflection, const char* name)
{
    for (const ShaderSamplerBindingInfo& binding : reflection.samplerBindings)
    {
        if (binding.name == name)
        {
            return &binding;
        }
    }
    return nullptr;
}

void applyNativeBinding(ResourceBinding& outBinding, const ShaderBufferBindingInfo* shaderBinding)
{
    if (shaderBinding)
    {
        outBinding.nativeBindingSlots = shaderBinding->nativeBindingSlots;
    }
}

void applyNativeBinding(ResourceBinding& outBinding, const ShaderTextureBindingInfo* shaderBinding)
{
    if (shaderBinding)
    {
        outBinding.nativeBindingSlots = shaderBinding->nativeBindingSlots;
    }
}

void applyNativeBinding(ResourceBinding& outBinding, const ShaderSamplerBindingInfo* shaderBinding)
{
    if (shaderBinding)
    {
        outBinding.nativeBindingSlots = shaderBinding->nativeBindingSlots;
    }
}

void applyNativeBufferBindings(ResourceBinding& outBinding,
    const ShaderReflectionDataEx& reflection,
    const char* name)
{
    bool found = false;
    for (const ShaderBufferBindingInfo& binding : reflection.bufferBindings)
    {
        if (binding.name != name)
        {
            continue;
        }

        found = true;
        if (binding.nativeBindingSlots.HasStageBinding(EShaderStage::Vertex))
        {
            outBinding.nativeBindingSlots.vertexBinding = binding.nativeBindingSlots.vertexBinding;
        }
        if (binding.nativeBindingSlots.HasStageBinding(EShaderStage::Fragment))
        {
            outBinding.nativeBindingSlots.fragmentBinding = binding.nativeBindingSlots.fragmentBinding;
        }
        if (binding.nativeBindingSlots.HasStageBinding(EShaderStage::Compute))
        {
            outBinding.nativeBindingSlots.computeBinding = binding.nativeBindingSlots.computeBinding;
        }
    }

    if (!found)
    {
        HS_LOG(warning, "[AtmospherePass] Missing reflected buffer binding for '%s'", name);
    }
}
}

AtmospherePass::~AtmospherePass()
{
    Shutdown();
}

bool AtmospherePass::Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext)
{
    _rhiContext = rhiContext;

    AtmosphereSettingsUBO defaultSettings = buildSettingsUBO(_settings);
    _lutResources.settingsBuffer = _rhiContext->CreateBuffer(
        "AtmosphereSettings",
        &defaultSettings,
        sizeof(defaultSettings),
        EBufferUsage::Uniform,
        EBufferMemoryOption::Dynamic);

    if (!_lutResources.settingsBuffer || !createLutResources())
    {
        HS_LOG(error, "[AtmospherePass] Failed to create LUT resources");
        return false;
    }

    if (!createComputeResources(shaderLibrary) || !createSkyShaders(shaderLibrary))
    {
        HS_LOG(error, "[AtmospherePass] Failed to create shaders or pipelines");
        return false;
    }

    _isInitialized = true;
    _settingsDirty = true;
    return true;
}

void AtmospherePass::Shutdown()
{
    if (!_rhiContext) return;

    _rhiContext->WaitForIdle();
    invalidateSkyPipelines();
    destroySkyResources();
    destroyComputeResources();
    destroyGpuResources();

    _isInitialized = false;
    _rhiContext = nullptr;
}

void AtmospherePass::UpdateSettings(const AtmosphereSettings& settings)
{
    if (!_lutResources.settingsBuffer) return;

    if (!_hasSettings || !settingsEqual(_settings, settings))
    {
        _settings = settings;
        _settingsDirty = true;
        _hasSettings = true;
    }

    AtmosphereSettingsUBO ubo = buildSettingsUBO(_settings);
    _rhiContext->UpdateBuffer(_lutResources.settingsBuffer, 0, &ubo, sizeof(ubo));
}

void AtmospherePass::PrecomputeIfNeeded(RHICommandBuffer& commandBuffer)
{
    if (!_isInitialized || !_settingsDirty || !_computeResourceSet)
    {
        return;
    }

    RHITextureBarrierDesc toStorage[] =
    {
        {_lutResources.transmittanceLut, ERHITextureState::ShaderRead, ERHITextureState::StorageReadWrite},
        {_lutResources.irradianceLut, ERHITextureState::ShaderRead, ERHITextureState::StorageReadWrite},
        {_lutResources.scatteringLut, ERHITextureState::ShaderRead, ERHITextureState::StorageReadWrite},
    };
    commandBuffer.TextureBarrier(toStorage, 3);

    for (ComputeStage& stage : _computeStages)
    {
        if (!stage.pipeline) continue;

        commandBuffer.BindComputePipeline(stage.pipeline);
        commandBuffer.BindComputeResourceSet(_computeResourceSet);
        commandBuffer.Dispatch(stage.groupX, stage.groupY, stage.groupZ);
        commandBuffer.EndComputePass();

        RHITextureBarrierDesc storageSync[] =
        {
            {_lutResources.transmittanceLut, ERHITextureState::StorageReadWrite, ERHITextureState::StorageReadWrite},
            {_lutResources.irradianceLut, ERHITextureState::StorageReadWrite, ERHITextureState::StorageReadWrite},
            {_lutResources.scatteringLut, ERHITextureState::StorageReadWrite, ERHITextureState::StorageReadWrite},
        };
        commandBuffer.TextureBarrier(storageSync, 3);
    }

    RHITextureBarrierDesc toShaderRead[] =
    {
        {_lutResources.transmittanceLut, ERHITextureState::StorageReadWrite, ERHITextureState::ShaderRead},
        {_lutResources.irradianceLut, ERHITextureState::StorageReadWrite, ERHITextureState::ShaderRead},
        {_lutResources.scatteringLut, ERHITextureState::StorageReadWrite, ERHITextureState::ShaderRead},
    };
    commandBuffer.TextureBarrier(toShaderRead, 3);

    _settingsDirty = false;
}

bool AtmospherePass::createLutResources()
{
    TextureInfo transInfo{};
    transInfo.type = ETextureType::Tex2D;
    transInfo.format = EPixelFormat::Rgba16f;
    transInfo.usage = ETextureUsage::Storage | ETextureUsage::Sampled;
    transInfo.extent.width = TransmittanceWidth;
    transInfo.extent.height = TransmittanceHeight;
    transInfo.extent.depth = 1;
    transInfo.byteSize = static_cast<size_t>(TransmittanceWidth) * TransmittanceHeight * Rgba16fBytesPerPixel;
    _lutResources.transmittanceLut = _rhiContext->CreateTexture("AtmosphereTransmittanceLut", nullptr, transInfo);

    TextureInfo irradianceInfo{};
    irradianceInfo.type = ETextureType::Tex2D;
    irradianceInfo.format = EPixelFormat::Rgba16f;
    irradianceInfo.usage = ETextureUsage::Storage | ETextureUsage::Sampled;
    irradianceInfo.extent.width = IrradianceWidth;
    irradianceInfo.extent.height = IrradianceHeight;
    irradianceInfo.extent.depth = 1;
    irradianceInfo.byteSize = static_cast<size_t>(IrradianceWidth) * IrradianceHeight * Rgba16fBytesPerPixel;
    _lutResources.irradianceLut = _rhiContext->CreateTexture("AtmosphereIrradianceLut", nullptr, irradianceInfo);

    TextureInfo scatteringInfo{};
    scatteringInfo.type = ETextureType::Tex3D;
    scatteringInfo.format = EPixelFormat::Rgba16f;
    scatteringInfo.usage = ETextureUsage::Storage | ETextureUsage::Sampled;
    scatteringInfo.extent.width = ScatteringWidth;
    scatteringInfo.extent.height = ScatteringHeight;
    scatteringInfo.extent.depth = ScatteringDepth;
    scatteringInfo.byteSize = static_cast<size_t>(ScatteringWidth) * ScatteringHeight * ScatteringDepth * Rgba16fBytesPerPixel;
    _lutResources.scatteringLut = _rhiContext->CreateTexture("AtmosphereScatteringLut", nullptr, scatteringInfo);

    SamplerInfo sampler2D{};
    sampler2D.type = ETextureType::Tex2D;
    sampler2D.minFilter = EFilterMode::Linear;
    sampler2D.magFilter = EFilterMode::Linear;
    sampler2D.mipmapMode = EFilterMode::Linear;
    sampler2D.addressU = EAddressMode::ClampToEdge;
    sampler2D.addressV = EAddressMode::ClampToEdge;
    sampler2D.addressW = EAddressMode::ClampToEdge;
    _lutResources.lutSampler2D = _rhiContext->CreateSampler("AtmosphereLutSampler2D", sampler2D);

    SamplerInfo sampler3D = sampler2D;
    sampler3D.type = ETextureType::Tex3D;
    _lutResources.lutSampler3D = _rhiContext->CreateSampler("AtmosphereLutSampler3D", sampler3D);

    return _lutResources.IsValid();
}

bool AtmospherePass::createComputeResources(ShaderLibrary* shaderLibrary)
{
    _computeStages =
    {
        ComputeStage{"AtmosphereTransmittance", nullptr, nullptr, (TransmittanceWidth + 7) / 8, (TransmittanceHeight + 7) / 8, 1},
        ComputeStage{"AtmosphereDirectIrradiance", nullptr, nullptr, (IrradianceWidth + 7) / 8, (IrradianceHeight + 7) / 8, 1},
        ComputeStage{"AtmosphereSingleScattering", nullptr, nullptr, (ScatteringWidth + 7) / 8, (ScatteringHeight + 7) / 8, (ScatteringDepth + 3) / 4},
        ComputeStage{"AtmosphereScatteringDensity", nullptr, nullptr, (ScatteringWidth + 7) / 8, (ScatteringHeight + 7) / 8, (ScatteringDepth + 3) / 4},
        ComputeStage{"AtmosphereIndirectIrradiance", nullptr, nullptr, (IrradianceWidth + 7) / 8, (IrradianceHeight + 7) / 8, 1},
        ComputeStage{"AtmosphereMultipleScattering", nullptr, nullptr, (ScatteringWidth + 7) / 8, (ScatteringHeight + 7) / 8, (ScatteringDepth + 3) / 4},
    };

    Shader* reflectionShader = nullptr;
    for (ComputeStage& stage : _computeStages)
    {
        Shader* shader = shaderLibrary->GetOrCompile(stage.shaderName, EShaderStage::Compute);
        if (!shader || !shader->IsCompiledEx())
        {
            HS_LOG(error, "[AtmospherePass] Failed to compile compute shader: %s", stage.shaderName);
            return false;
        }

        if (!reflectionShader)
        {
            reflectionShader = shader;
        }

        const auto* bytecode = shader->GetByteCode(EShaderStage::Compute);
        if (!bytecode)
        {
            HS_LOG(error, "[AtmospherePass] Missing compute bytecode: %s", stage.shaderName);
            return false;
        }

        ShaderInfo shaderInfo{};
        shaderInfo.stage = EShaderStage::Compute;
        shaderInfo.entryName = shader->GetEntryPoint(EShaderStage::Compute);
        stage.shader = _rhiContext->CreateShader(
            stage.shaderName,
            shaderInfo,
            reinterpret_cast<const char*>(bytecode->data()),
            bytecode->size());
        if (!stage.shader) return false;
    }

    const ShaderReflectionDataEx& reflection = reflectionShader->GetReflection();

    ResourceBinding settingsBinding{};
    settingsBinding.type = EResourceType::UniformBuffer;
    settingsBinding.stage = EShaderStage::Compute;
    settingsBinding.binding = 0;
    settingsBinding.arrayCount = 1;
    applyNativeBinding(settingsBinding, findBufferBinding(reflection, "atmosphereSettings"));
    settingsBinding.resource.buffers.push_back(_lutResources.settingsBuffer);
    settingsBinding.resource.offsets.push_back(0);

    ResourceBinding transStorage{};
    transStorage.type = EResourceType::StorageImage;
    transStorage.stage = EShaderStage::Compute;
    transStorage.binding = 1;
    transStorage.arrayCount = 1;
    applyNativeBinding(transStorage, findTextureBinding(reflection, "transmittanceLut"));
    transStorage.resource.textures.push_back(_lutResources.transmittanceLut);

    ResourceBinding irradianceStorage = transStorage;
    irradianceStorage.binding = 2;
    applyNativeBinding(irradianceStorage, findTextureBinding(reflection, "irradianceLut"));
    irradianceStorage.resource.textures.clear();
    irradianceStorage.resource.textures.push_back(_lutResources.irradianceLut);

    ResourceBinding scatteringStorage = transStorage;
    scatteringStorage.binding = 3;
    applyNativeBinding(scatteringStorage, findTextureBinding(reflection, "scatteringLut"));
    scatteringStorage.resource.textures.clear();
    scatteringStorage.resource.textures.push_back(_lutResources.scatteringLut);

    ResourceBinding bindings[4] =
    {
        settingsBinding,
        transStorage,
        irradianceStorage,
        scatteringStorage
    };
    _computeResourceLayout = _rhiContext->CreateResourceLayout("AtmosphereComputeLayout", bindings, 4);
    if (!_computeResourceLayout) return false;

    _computeResourceSet = _rhiContext->CreateResourceSet("AtmosphereComputeSet", _computeResourceLayout);
    if (!_computeResourceSet) return false;

    for (ComputeStage& stage : _computeStages)
    {
        ComputePipelineInfo pipelineInfo{};
        pipelineInfo.computeShader = stage.shader;
        pipelineInfo.resourceLayout = _computeResourceLayout;
        stage.pipeline = _rhiContext->CreateComputePipeline(stage.shaderName, pipelineInfo);
        if (!stage.pipeline) return false;
    }

    return true;
}

bool AtmospherePass::createSkyShaders(ShaderLibrary* shaderLibrary)
{
    Shader* shader = shaderLibrary->GetOrCompile("AtmosphereSky", EShaderStage::Vertex | EShaderStage::Fragment);
    if (!shader || !shader->IsCompiledEx())
    {
        HS_LOG(error, "[AtmospherePass] AtmosphereSky shader compilation failed");
        return false;
    }

    const auto* vsBytecode = shader->GetByteCode(EShaderStage::Vertex);
    const auto* fsBytecode = shader->GetByteCode(EShaderStage::Fragment);
    if (!vsBytecode || !fsBytecode) return false;

    _skyReflection = shader->GetReflection();

    ShaderInfo vsInfo{};
    vsInfo.stage = EShaderStage::Vertex;
    vsInfo.entryName = shader->GetEntryPoint(EShaderStage::Vertex);
    _skyVertexShader = _rhiContext->CreateShader(
        "AtmosphereSkyVS", vsInfo,
        reinterpret_cast<const char*>(vsBytecode->data()),
        vsBytecode->size());

    ShaderInfo fsInfo{};
    fsInfo.stage = EShaderStage::Fragment;
    fsInfo.entryName = shader->GetEntryPoint(EShaderStage::Fragment);
    _skyFragmentShader = _rhiContext->CreateShader(
        "AtmosphereSkyFS", fsInfo,
        reinterpret_cast<const char*>(fsBytecode->data()),
        fsBytecode->size());

    return _skyVertexShader && _skyFragmentShader;
}

void AtmospherePass::destroyGpuResources()
{
    if (_lutResources.lutSampler3D)
    {
        _rhiContext->DestroySampler(_lutResources.lutSampler3D);
        _lutResources.lutSampler3D = nullptr;
    }
    if (_lutResources.lutSampler2D)
    {
        _rhiContext->DestroySampler(_lutResources.lutSampler2D);
        _lutResources.lutSampler2D = nullptr;
    }
    if (_lutResources.scatteringLut)
    {
        _rhiContext->DestroyTexture(_lutResources.scatteringLut);
        _lutResources.scatteringLut = nullptr;
    }
    if (_lutResources.irradianceLut)
    {
        _rhiContext->DestroyTexture(_lutResources.irradianceLut);
        _lutResources.irradianceLut = nullptr;
    }
    if (_lutResources.transmittanceLut)
    {
        _rhiContext->DestroyTexture(_lutResources.transmittanceLut);
        _lutResources.transmittanceLut = nullptr;
    }
    if (_lutResources.settingsBuffer)
    {
        _rhiContext->DestroyBuffer(_lutResources.settingsBuffer);
        _lutResources.settingsBuffer = nullptr;
    }
}

void AtmospherePass::destroyComputeResources()
{
    for (ComputeStage& stage : _computeStages)
    {
        if (stage.pipeline)
        {
            _rhiContext->DestroyComputePipeline(stage.pipeline);
            stage.pipeline = nullptr;
        }
        if (stage.shader)
        {
            _rhiContext->DestroyShader(stage.shader);
            stage.shader = nullptr;
        }
    }

    if (_computeResourceSet)
    {
        _rhiContext->DestroyResourceSet(_computeResourceSet);
        _computeResourceSet = nullptr;
    }
    if (_computeResourceLayout)
    {
        _rhiContext->DestroyResourceLayout(_computeResourceLayout);
        _computeResourceLayout = nullptr;
    }
}

void AtmospherePass::destroySkyResources()
{
    if (_skyResourceSet)
    {
        _rhiContext->DestroyResourceSet(_skyResourceSet);
        _skyResourceSet = nullptr;
    }
    if (_skyResourceLayout)
    {
        _rhiContext->DestroyResourceLayout(_skyResourceLayout);
        _skyResourceLayout = nullptr;
    }
    if (_skyFragmentShader)
    {
        _rhiContext->DestroyShader(_skyFragmentShader);
        _skyFragmentShader = nullptr;
    }
    if (_skyVertexShader)
    {
        _rhiContext->DestroyShader(_skyVertexShader);
        _skyVertexShader = nullptr;
    }
    _perViewBuffer = nullptr;
}

void AtmospherePass::invalidateSkyPipelines()
{
    for (auto& [key, pipeline] : _skyPipelineCache)
    {
        if (pipeline)
        {
            _rhiContext->DestroyGraphicsPipeline(pipeline);
        }
    }
    _skyPipelineCache.clear();
}

void AtmospherePass::rebuildSkyResourceBindings(RHIBuffer* perViewBuffer)
{
    if (_skyResourceSet)
    {
        _rhiContext->DestroyResourceSet(_skyResourceSet);
        _skyResourceSet = nullptr;
    }
    if (_skyResourceLayout)
    {
        _rhiContext->DestroyResourceLayout(_skyResourceLayout);
        _skyResourceLayout = nullptr;
    }

    ResourceBinding perViewBinding{};
    perViewBinding.type = EResourceType::UniformBuffer;
    perViewBinding.stage = EShaderStage::Vertex | EShaderStage::Fragment;
    perViewBinding.binding = 0;
    perViewBinding.arrayCount = 1;
    applyNativeBufferBindings(perViewBinding, _skyReflection, "perView");
    perViewBinding.resource.buffers.push_back(perViewBuffer);
    perViewBinding.resource.offsets.push_back(0);

    ResourceBinding settingsBinding{};
    settingsBinding.type = EResourceType::UniformBuffer;
    settingsBinding.stage = EShaderStage::Fragment;
    settingsBinding.binding = 1;
    settingsBinding.arrayCount = 1;
    applyNativeBufferBindings(settingsBinding, _skyReflection, "atmosphereSettings");
    settingsBinding.resource.buffers.push_back(_lutResources.settingsBuffer);
    settingsBinding.resource.offsets.push_back(0);

    ResourceBinding transBinding{};
    transBinding.type = EResourceType::SampledImage;
    transBinding.stage = EShaderStage::Fragment;
    transBinding.binding = 2;
    transBinding.arrayCount = 1;
    applyNativeBinding(transBinding, findTextureBinding(_skyReflection, "transmittanceLut"));
    transBinding.resource.textures.push_back(_lutResources.transmittanceLut);

    ResourceBinding irradianceBinding = transBinding;
    irradianceBinding.binding = 3;
    applyNativeBinding(irradianceBinding, findTextureBinding(_skyReflection, "irradianceLut"));
    irradianceBinding.resource.textures.clear();
    irradianceBinding.resource.textures.push_back(_lutResources.irradianceLut);

    ResourceBinding scatteringBinding = transBinding;
    scatteringBinding.binding = 4;
    applyNativeBinding(scatteringBinding, findTextureBinding(_skyReflection, "scatteringLut"));
    scatteringBinding.resource.textures.clear();
    scatteringBinding.resource.textures.push_back(_lutResources.scatteringLut);

    ResourceBinding sampler2DBinding{};
    sampler2DBinding.type = EResourceType::Sampler;
    sampler2DBinding.stage = EShaderStage::Fragment;
    sampler2DBinding.binding = 5;
    sampler2DBinding.arrayCount = 1;
    applyNativeBinding(sampler2DBinding, findSamplerBinding(_skyReflection, "lutSampler2D"));
    sampler2DBinding.resource.samplers.push_back(_lutResources.lutSampler2D);

    ResourceBinding sampler3DBinding{};
    sampler3DBinding.type = EResourceType::Sampler;
    sampler3DBinding.stage = EShaderStage::Fragment;
    sampler3DBinding.binding = 6;
    sampler3DBinding.arrayCount = 1;
    applyNativeBinding(sampler3DBinding, findSamplerBinding(_skyReflection, "lutSampler3D"));
    sampler3DBinding.resource.samplers.push_back(_lutResources.lutSampler3D);

    ResourceBinding bindings[7] =
    {
        perViewBinding,
        settingsBinding,
        transBinding,
        irradianceBinding,
        scatteringBinding,
        sampler2DBinding,
        sampler3DBinding
    };
    _skyResourceLayout = _rhiContext->CreateResourceLayout("AtmosphereSkyLayout", bindings, 7);
    if (!_skyResourceLayout)
    {
        HS_LOG(error, "[AtmospherePass] Failed to create sky resource layout");
        return;
    }

    _skyResourceSet = _rhiContext->CreateResourceSet("AtmosphereSkySet", _skyResourceLayout);
    if (!_skyResourceSet)
    {
        HS_LOG(error, "[AtmospherePass] Failed to create sky resource set");
        return;
    }

    _perViewBuffer = perViewBuffer;
}

RHIGraphicsPipeline* AtmospherePass::GetOrCreateSkyPipeline(
    const PipelineRenderTargetLayout& renderTargetLayout,
    RHIBuffer* perViewBuffer)
{
    if (!_isInitialized || !perViewBuffer) return nullptr;

    if (perViewBuffer != _perViewBuffer)
    {
        rebuildSkyResourceBindings(perViewBuffer);
        invalidateSkyPipelines();
    }

    if (!_skyResourceLayout || !_skyResourceSet) return nullptr;

    size_t pipelineKey = std::hash<PipelineRenderTargetLayout>{}(renderTargetLayout);
    auto it = _skyPipelineCache.find(pipelineKey);
    if (it != _skyPipelineCache.end())
    {
        return it->second;
    }

    VertexInputStateDescriptor viDesc{};

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
    dsDesc.depthTestEnable = renderTargetLayout.useDepthStencilAttachment;
    dsDesc.depthWriteEnable = false;
    dsDesc.depthCompareOp = ECompareOp::LessOrEqual;
    dsDesc.depthBoundTestEnable = false;
    dsDesc.stencilTestEnable = false;

    ColorBlendStateDescriptor cbDesc{};
    cbDesc.logicOpEnable = false;
    cbDesc.attachmentCount = renderTargetLayout.colorAttachmentCount;
    cbDesc.attachments.resize(cbDesc.attachmentCount);
    for (ColorBlendAttachmentDescriptor& att : cbDesc.attachments)
    {
        att.blendEnable = false;
        att.srcColorFactor = EBlendFactor::One;
        att.dstColorFactor = EBlendFactor::Zero;
        att.colorBlendOp = EBlendOp::Add;
        att.srcAlphaFactor = EBlendFactor::One;
        att.dstAlphaFactor = EBlendFactor::Zero;
        att.alphaBlendOp = EBlendOp::Add;
    }

    ShaderProgramDescriptor spDesc{};
    spDesc.stages.push_back(_skyVertexShader);
    spDesc.stages.push_back(_skyFragmentShader);

    GraphicsPipelineInfo gpInfo{};
    gpInfo.shaderDesc = spDesc;
    gpInfo.inputAssemblyDesc = iaDesc;
    gpInfo.vertexInputDesc = viDesc;
    gpInfo.rasterizerDesc = rsDesc;
    gpInfo.depthStencilDesc = dsDesc;
    gpInfo.colorBlendDesc = cbDesc;
    gpInfo.renderTargetLayout = renderTargetLayout;
    gpInfo.resourceLayout = _skyResourceLayout;

    RHIGraphicsPipeline* pipeline = _rhiContext->CreateGraphicsPipeline("AtmosphereSkyPipeline", gpInfo);
    if (pipeline)
    {
        _skyPipelineCache[pipelineKey] = pipeline;
    }
    return pipeline;
}

bool AtmospherePass::settingsEqual(const AtmosphereSettings& lhs, const AtmosphereSettings& rhs) const
{
    return lhs.enabled == rhs.enabled &&
           nearEqual(lhs.sunDirection.x, rhs.sunDirection.x) &&
           nearEqual(lhs.sunDirection.y, rhs.sunDirection.y) &&
           nearEqual(lhs.sunDirection.z, rhs.sunDirection.z) &&
           nearEqual(lhs.sunIntensity, rhs.sunIntensity) &&
           nearEqual(lhs.exposure, rhs.exposure) &&
           nearEqual(lhs.rayleighMultiplier, rhs.rayleighMultiplier) &&
           nearEqual(lhs.mieMultiplier, rhs.mieMultiplier) &&
           nearEqual(lhs.ozoneMultiplier, rhs.ozoneMultiplier) &&
           nearEqual(lhs.mieG, rhs.mieG) &&
           nearEqual(lhs.planetRadiusMeters, rhs.planetRadiusMeters) &&
           nearEqual(lhs.atmosphereRadiusMeters, rhs.atmosphereRadiusMeters) &&
           nearEqual(lhs.rayleighScaleHeightMeters, rhs.rayleighScaleHeightMeters) &&
           nearEqual(lhs.mieScaleHeightMeters, rhs.mieScaleHeightMeters) &&
           lhs.multipleScatteringOrder == rhs.multipleScatteringOrder &&
           lhs.debugView == rhs.debugView;
}

AtmospherePass::AtmosphereSettingsUBO AtmospherePass::buildSettingsUBO(const AtmosphereSettings& settings) const
{
    glm::vec3 sun = glm::length(settings.sunDirection) > 0.0001f
        ? glm::normalize(settings.sunDirection)
        : glm::vec3(0.0f, 1.0f, 0.0f);

    AtmosphereSettingsUBO ubo{};
    ubo.planetRadii = glm::vec4(
        settings.planetRadiusMeters,
        settings.atmosphereRadiusMeters,
        std::max(settings.atmosphereRadiusMeters - settings.planetRadiusMeters, 1.0f),
        0.0f);
    ubo.densityParams = glm::vec4(
        settings.rayleighScaleHeightMeters,
        settings.mieScaleHeightMeters,
        settings.rayleighMultiplier,
        settings.mieMultiplier);
    ubo.scatteringParams = glm::vec4(
        settings.ozoneMultiplier,
        settings.mieG,
        0.0f,
        0.0f);
    ubo.sunDirectionIntensity = glm::vec4(sun, settings.sunIntensity);
    ubo.debugExposureOrder = glm::vec4(
        static_cast<float>(settings.debugView),
        settings.exposure,
        static_cast<float>(std::clamp(settings.multipleScatteringOrder, 1u, 8u)),
        settings.enabled ? 1.0f : 0.0f);
    return ubo;
}

HS_NS_END
