#include "Renderer/RenderPass/ForwardSkyboxPass.h"

#include "Core/Log.h"
#include "Core/Hash.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"

#include "Renderer/ShaderLibrary.h"

#include "Resource/Image.h"
#include "Resource/Shader.h"

#include <cstring>
#include <vector>

HS_NS_BEGIN

ForwardSkyboxPass::~ForwardSkyboxPass()
{
    Shutdown();
}

bool ForwardSkyboxPass::Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext)
{
    _rhiContext = rhiContext;

    Shader* shader = shaderLibrary->GetOrCompile("Skybox", EShaderStage::Vertex | EShaderStage::Fragment);
    if (!shader || !shader->IsCompiledEx())
    {
        HS_LOG(error, "[ForwardSkyboxPass] Skybox shader compilation failed");
        return false;
    }

    const auto* vsBytecode = shader->GetByteCode(EShaderStage::Vertex);
    const auto* fsBytecode = shader->GetByteCode(EShaderStage::Fragment);
    if (!vsBytecode || !fsBytecode)
    {
        HS_LOG(error, "[ForwardSkyboxPass] Skybox shader bytecode not found");
        return false;
    }

    ShaderInfo vsInfo{};
    vsInfo.stage     = EShaderStage::Vertex;
    vsInfo.entryName = shader->GetEntryPoint(EShaderStage::Vertex);
    _vertexShader    = _rhiContext->CreateShader(
        "SkyboxVS", vsInfo,
        reinterpret_cast<const char*>(vsBytecode->data()),
        vsBytecode->size()
    );

    ShaderInfo fsInfo{};
    fsInfo.stage     = EShaderStage::Fragment;
    fsInfo.entryName = shader->GetEntryPoint(EShaderStage::Fragment);
    _fragmentShader  = _rhiContext->CreateShader(
        "SkyboxFS", fsInfo,
        reinterpret_cast<const char*>(fsBytecode->data()),
        fsBytecode->size()
    );

    if (!_vertexShader || !_fragmentShader)
    {
        HS_LOG(error, "[ForwardSkyboxPass] Failed to create Skybox RHI shaders");
        return false;
    }

    _isInitialized = true;
    return true;
}

void ForwardSkyboxPass::Shutdown()
{
    if (!_rhiContext) return;

    _rhiContext->WaitForIdle();

    for (auto& [key, pipeline] : _pipelineCache)
    {
        if (pipeline)
        {
            _rhiContext->DestroyGraphicsPipeline(pipeline);
        }
    }
    _pipelineCache.clear();

    destroyCubemapResources();

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

    _perViewBuffer = nullptr;
    _isInitialized = false;
    _rhiContext    = nullptr;
}

void ForwardSkyboxPass::destroyCubemapResources()
{
    if (!_rhiContext) return;

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
    if (_cubemapSampler)
    {
        _rhiContext->DestroySampler(_cubemapSampler);
        _cubemapSampler = nullptr;
    }
    if (_cubemapTexture)
    {
        _rhiContext->DestroyTexture(_cubemapTexture);
        _cubemapTexture = nullptr;
    }
    _mode = ESkyboxMode::None;
}

bool ForwardSkyboxPass::ConfigureSixSided(const std::array<Image*, 6>& faces)
{
    if (!_isInitialized)
    {
        HS_LOG(error, "[ForwardSkyboxPass] Must call Initialize() first");
        return false;
    }

    for (uint32 i = 0; i < 6; ++i)
    {
        if (!faces[i] || faces[i]->GetRawData() == nullptr || faces[i]->GetRawDataSize() == 0)
        {
            HS_LOG(error, "[ForwardSkyboxPass] Face[%u] image is invalid", i);
            return false;
        }
    }

    uint16 faceWidth  = faces[0]->GetWidth();
    uint16 faceHeight = faces[0]->GetHeight();
    for (uint32 i = 1; i < 6; ++i)
    {
        if (faces[i]->GetWidth() != faceWidth || faces[i]->GetHeight() != faceHeight)
        {
            HS_LOG(error, "[ForwardSkyboxPass] All faces must have identical dimensions");
            return false;
        }
    }

    destroyCubemapResources();

    size_t faceByteSize = faces[0]->GetRawDataSize();
    size_t totalSize    = faceByteSize * 6;

    std::vector<uint8> combinedData(totalSize);
    for (uint32 i = 0; i < 6; ++i)
    {
        size_t faceSize = faces[i]->GetRawDataSize();
        if (faceSize != faceByteSize)
        {
            HS_LOG(error, "[ForwardSkyboxPass] Face[%u] byte size mismatch", i);
            return false;
        }
        ::memcpy(combinedData.data() + i * faceByteSize, faces[i]->GetRawData(), faceByteSize);
    }

    TextureInfo texInfo{};
    texInfo.type          = ETextureType::TexCube;
    texInfo.format        = EPixelFormat::R8G8B8A8Unorm;
    texInfo.usage         = ETextureUsage::Sampled;
    texInfo.extent.width  = faceWidth;
    texInfo.extent.height = faceHeight;
    texInfo.extent.depth  = 1;
    texInfo.mipLevel      = 1;
    texInfo.arrayLength   = 1;
    texInfo.byteSize      = totalSize;

    _cubemapTexture = _rhiContext->CreateTexture("SkyboxCubemap", combinedData.data(), texInfo);
    if (!_cubemapTexture)
    {
        HS_LOG(error, "[ForwardSkyboxPass] Failed to create cubemap RHI texture");
        return false;
    }

    SamplerInfo samplerInfo{};
    samplerInfo.type       = ETextureType::TexCube;
    samplerInfo.minFilter  = EFilterMode::Linear;
    samplerInfo.magFilter  = EFilterMode::Linear;
    samplerInfo.mipmapMode = EFilterMode::Linear;
    samplerInfo.addressU   = EAddressMode::ClampToEdge;
    samplerInfo.addressV   = EAddressMode::ClampToEdge;
    samplerInfo.addressW   = EAddressMode::ClampToEdge;

    _cubemapSampler = _rhiContext->CreateSampler("SkyboxSampler", samplerInfo);
    if (!_cubemapSampler)
    {
        HS_LOG(error, "[ForwardSkyboxPass] Failed to create cubemap sampler");
        return false;
    }

    _mode = ESkyboxMode::SixSided;
    _perViewBuffer = nullptr;

    for (auto& [key, pipeline] : _pipelineCache)
    {
        if (pipeline)
        {
            _rhiContext->DestroyGraphicsPipeline(pipeline);
        }
    }
    _pipelineCache.clear();

    return true;
}

void ForwardSkyboxPass::rebuildResourceBindings(RHIBuffer* perViewBuffer)
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

    ResourceBinding perViewBinding{};
    perViewBinding.type       = EResourceType::UniformBuffer;
    perViewBinding.stage      = EShaderStage::Vertex | EShaderStage::Fragment;
    perViewBinding.binding    = 0;
    perViewBinding.arrayCount = 1;
    perViewBinding.resource.buffers.push_back(perViewBuffer);
    perViewBinding.resource.offsets.push_back(0);

    ResourceBinding cubemapBinding{};
    cubemapBinding.type       = EResourceType::CombinedImageSampler;
    cubemapBinding.stage      = EShaderStage::Fragment;
    cubemapBinding.binding    = 1;
    cubemapBinding.arrayCount = 1;
    cubemapBinding.resource.textures.push_back(_cubemapTexture);
    cubemapBinding.resource.samplers.push_back(_cubemapSampler);

    ResourceBinding bindings[2] = { perViewBinding, cubemapBinding };
    _resourceLayout = _rhiContext->CreateResourceLayout("SkyboxLayout", bindings, 2);
    if (!_resourceLayout)
    {
        HS_LOG(error, "[ForwardSkyboxPass] Failed to create ResourceLayout");
        return;
    }

    _resourceSet = _rhiContext->CreateResourceSet("SkyboxSet", _resourceLayout);
    if (!_resourceSet)
    {
        HS_LOG(error, "[ForwardSkyboxPass] Failed to create ResourceSet");
        return;
    }

    _perViewBuffer = perViewBuffer;
}

RHIGraphicsPipeline* ForwardSkyboxPass::GetOrCreatePipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                                             RHIBuffer* perViewBuffer)
{
    if (!_isInitialized || !HasSkybox()) return nullptr;

    if (perViewBuffer != _perViewBuffer)
    {
        rebuildResourceBindings(perViewBuffer);

        for (auto& [key, pipeline] : _pipelineCache)
        {
            if (pipeline)
            {
                _rhiContext->DestroyGraphicsPipeline(pipeline);
            }
        }
        _pipelineCache.clear();
    }

    if (!_resourceLayout || !_resourceSet) return nullptr;

    size_t pipelineKey = std::hash<PipelineRenderTargetLayout>{}(renderTargetLayout);

    auto it = _pipelineCache.find(pipelineKey);
    if (it != _pipelineCache.end())
    {
        return it->second;
    }

    VertexInputStateDescriptor viDesc{};

    InputAssemblyStateDescriptor iaDesc{};
    iaDesc.primitiveTopology = EPrimitiveTopology::TriangleList;
    iaDesc.isRestartEnable   = false;

    RasterizerStateDescriptor rsDesc{};
    rsDesc.cullMode                = ECullMode::None;
    rsDesc.frontFace               = EFrontFace::CounterClockwise;
    rsDesc.polygonMode             = EPolygonMode::Fill;
    rsDesc.depthClampEnable        = false;
    rsDesc.rasterizerDiscardEnable = false;
    rsDesc.depthBiasEnable         = false;
    rsDesc.lineWidth               = 1.0f;

    DepthStencilStateDescriptor dsDesc{};
    dsDesc.depthTestEnable      = renderTargetLayout.useDepthStencilAttachment;
    dsDesc.depthWriteEnable     = false;
    dsDesc.depthCompareOp       = ECompareOp::LessOrEqual;
    dsDesc.depthBoundTestEnable = false;
    dsDesc.stencilTestEnable    = false;

    ColorBlendStateDescriptor cbDesc{};
    cbDesc.logicOpEnable   = false;
    cbDesc.attachmentCount = renderTargetLayout.colorAttachmentCount;
    cbDesc.attachments.resize(cbDesc.attachmentCount);
    for (size_t i = 0; i < cbDesc.attachmentCount; ++i)
    {
        ColorBlendAttachmentDescriptor& att = cbDesc.attachments[i];
        att.blendEnable    = false;
        att.srcColorFactor = EBlendFactor::One;
        att.dstColorFactor = EBlendFactor::Zero;
        att.colorBlendOp   = EBlendOp::Add;
        att.srcAlphaFactor = EBlendFactor::One;
        att.dstAlphaFactor = EBlendFactor::Zero;
        att.alphaBlendOp   = EBlendOp::Add;
    }

    ShaderProgramDescriptor spDesc{};
    spDesc.stages.push_back(_vertexShader);
    spDesc.stages.push_back(_fragmentShader);

    GraphicsPipelineInfo gpInfo{};
    gpInfo.shaderDesc         = spDesc;
    gpInfo.inputAssemblyDesc  = iaDesc;
    gpInfo.vertexInputDesc    = viDesc;
    gpInfo.rasterizerDesc     = rsDesc;
    gpInfo.depthStencilDesc   = dsDesc;
    gpInfo.colorBlendDesc     = cbDesc;
    gpInfo.renderTargetLayout = renderTargetLayout;
    gpInfo.resourceLayout     = _resourceLayout;

    RHIGraphicsPipeline* pipeline = _rhiContext->CreateGraphicsPipeline("SkyboxPipeline", gpInfo);
    if (pipeline)
    {
        _pipelineCache[pipelineKey] = pipeline;
    }
    else
    {
        HS_LOG(error, "[ForwardSkyboxPass] Failed to create GraphicsPipeline");
    }

    return pipeline;
}

HS_NS_END
