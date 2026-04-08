#include "Renderer/RenderPass/ForwardGridPass.h"

#include "Core/Log.h"
#include "Core/Hash.h"

#include "RHI/RHIContext.h"
#include "RHI/RenderHandle.h"
#include "RHI/ResourceHandle.h"

#include "Renderer/ShaderLibrary.h"

#include "Resource/Shader.h"

HS_NS_BEGIN

ForwardGridPass::~ForwardGridPass()
{
    Shutdown();
}

bool ForwardGridPass::Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext)
{
    _rhiContext = rhiContext;

    Shader* shader = shaderLibrary->GetOrCompile("GridDepth", EShaderStage::Vertex | EShaderStage::Fragment);
    if (!shader || !shader->IsCompiledEx())
    {
        HS_LOG(error, "[ForwardGridPass] Grid 셰이더 컴파일 실패 — Grid.slang 확인 필요");
        return false;
    }

    const auto* vsBytecode = shader->GetBytecode(EShaderStage::Vertex);
    const auto* fsBytecode = shader->GetBytecode(EShaderStage::Fragment);
    if (!vsBytecode || !fsBytecode)
    {
        HS_LOG(error, "[ForwardGridPass] Grid 셰이더 바이트코드 없음");
        return false;
    }

    ShaderInfo vsInfo{};
    vsInfo.stage     = EShaderStage::Vertex;
    vsInfo.entryName = shader->GetEntryPoint(EShaderStage::Vertex);
    _vertexShader    = _rhiContext->CreateShader(
        "GridVS", vsInfo,
        reinterpret_cast<const char*>(vsBytecode->data()),
        vsBytecode->size()
    );

    ShaderInfo fsInfo{};
    fsInfo.stage     = EShaderStage::Fragment;
    fsInfo.entryName = shader->GetEntryPoint(EShaderStage::Fragment);
    _fragmentShader  = _rhiContext->CreateShader(
        "GridFS", fsInfo,
        reinterpret_cast<const char*>(fsBytecode->data()),
        fsBytecode->size()
    );

    if (!_vertexShader || !_fragmentShader)
    {
        HS_LOG(error, "[ForwardGridPass] Grid RHI 셰이더 생성 실패");
        return false;
    }

    _isInitialized = true;
    return true;
}

void ForwardGridPass::Shutdown()
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

    _perViewBuffer = nullptr;
    _isInitialized = false;
    _rhiContext    = nullptr;
}

void ForwardGridPass::rebuildResourceBindings(RHIBuffer* perViewBuffer)
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

    ResourceBinding binding{};
    binding.type       = EResourceType::UniformBuffer;
    binding.stage      = EShaderStage::Vertex | EShaderStage::Fragment;
    binding.binding    = 0;
    binding.arrayCount = 1;
    binding.resource.buffers.push_back(perViewBuffer);
    binding.resource.offsets.push_back(0); // Buffer면 Offset도 함께

    _resourceLayout = _rhiContext->CreateResourceLayout("GridLayout", &binding, 1);
    if (!_resourceLayout)
    {
        HS_LOG(error, "[ForwardGridPass] ResourceLayout 생성 실패");
        return;
    }

    _resourceSet = _rhiContext->CreateResourceSet("GridSet", _resourceLayout);
    if (!_resourceSet)
    {
        HS_LOG(error, "[ForwardGridPass] ResourceSet 생성 실패");
        return;
    }

    _perViewBuffer = perViewBuffer;
}

RHIGraphicsPipeline* ForwardGridPass::GetOrCreatePipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                                          RHIBuffer* perViewBuffer)
{
    if (!_isInitialized) return nullptr;

    // perViewBuffer가 바뀌면 ResourceBinding 재생성 + 파이프라인 캐시 무효화
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

    // 빈 VertexInput — fullscreen triangle은 버텍스 버퍼를 사용하지 않습니다.
    VertexInputStateDescriptor viDesc{};

    InputAssemblyStateDescriptor iaDesc{};
    iaDesc.primitiveTopology = EPrimitiveTopology::TriangleList;
    iaDesc.isRestartEnable   = false;

    RasterizerStateDescriptor rsDesc{};
    rsDesc.cullMode                = ECullMode::None; // 양면 렌더
    rsDesc.frontFace               = EFrontFace::CounterClockwise;
    rsDesc.polygonMode             = EPolygonMode::Fill;
    rsDesc.depthClampEnable        = false;
    rsDesc.rasterizerDiscardEnable = false;
    rsDesc.depthBiasEnable         = false;
    rsDesc.lineWidth               = 1.0f;

    DepthStencilStateDescriptor dsDesc{};
    dsDesc.depthTestEnable      = true;
    dsDesc.depthWriteEnable     = true; // 격자가 Y=0 실제 평면처럼 깊이 경쟁
    dsDesc.depthCompareOp       = ECompareOp::Less;
    dsDesc.depthBoundTestEnable = false;
    dsDesc.stencilTestEnable    = false;

    ColorBlendStateDescriptor cbDesc{};
    cbDesc.logicOpEnable   = false;
    cbDesc.attachmentCount = renderTargetLayout.colorAttachmentCount;
    cbDesc.attachments.resize(cbDesc.attachmentCount);
    for (size_t i = 0; i < cbDesc.attachmentCount; ++i)
    {
        ColorBlendAttachmentDescriptor& att = cbDesc.attachments[i];
        att.blendEnable    = true;
        att.srcColorFactor = EBlendFactor::SrcAlpha;
        att.dstColorFactor = EBlendFactor::OneMinusSrcAlpha;
        att.colorBlendOp   = EBlendOp::Add;
        att.srcAlphaFactor = EBlendFactor::One;
        att.dstAlphaFactor = EBlendFactor::OneMinusSrcAlpha;
        att.alphaBlendOp   = EBlendOp::Add;
    }

    ShaderProgramDescriptor spDesc{};
    spDesc.stages.push_back(_vertexShader);
    spDesc.stages.push_back(_fragmentShader);

    GraphicsPipelineInfo gpInfo{};
    gpInfo.shaderDesc        = spDesc;
    gpInfo.inputAssemblyDesc = iaDesc;
    gpInfo.vertexInputDesc   = viDesc;
    gpInfo.rasterizerDesc    = rsDesc;
    gpInfo.depthStencilDesc  = dsDesc;
    gpInfo.colorBlendDesc    = cbDesc;
    gpInfo.renderTargetLayout = renderTargetLayout;
    gpInfo.resourceLayout    = _resourceLayout;

    RHIGraphicsPipeline* pipeline = _rhiContext->CreateGraphicsPipeline("GridPipeline", gpInfo);
    if (pipeline)
    {
        _pipelineCache[pipelineKey] = pipeline;
    }
    else
    {
        HS_LOG(error, "[ForwardGridPass] GraphicsPipeline 생성 실패");
    }

    return pipeline;
}

HS_NS_END
