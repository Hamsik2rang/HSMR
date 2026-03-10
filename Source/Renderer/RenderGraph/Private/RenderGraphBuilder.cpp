#include "Renderer/RenderGraph/RenderGraphBuilder.h"

HS_NS_BEGIN

RGPass::~RGPass()
{
    _upstreams.clear();
    _downstreams.clear();
    _params     = {};
    _isExecuted = false;
    _isCompiled = false;
    _isCulled   = true;
    _fnExecute  = nullptr;
}

RenderGraphBuilder::RenderGraphBuilder()
{
}

RenderGraphBuilder::~RenderGraphBuilder()
{
}

RGTexture* RenderGraphBuilder::RegisterExternalTexture(RHITexture* texture)
{
    return nullptr;
}

void RenderGraphBuilder::UnregisterExternalTexture(RHITexture* texture)
{
}

RGTexture* RenderGraphBuilder::AcquireTexture(const RGTextureDescriptor& desc)
{
    return nullptr;
}

RGTexture* RenderGraphBuilder::FindTexture(RHITexture* texture) const
{
    return nullptr;
}

RGTexture* RenderGraphBuilder::FindTexture(uint32 id) const
{
    return nullptr;
}

RGBuffer* RenderGraphBuilder::RegisterExternalBuffer(RHIBuffer* buffer)
{
    return nullptr;
}

void RenderGraphBuilder::UnregisterExternalBuffer(RHIBuffer* buffer)
{
}

RGBuffer* RenderGraphBuilder::AcquireBuffer(const RGBufferDescriptor& desc)
{
    return nullptr;
}

RGBuffer* RenderGraphBuilder::FindBuffer(RHIBuffer* buffer) const
{
    return nullptr;
}

void RenderGraphBuilder::AddPass(const char* passName, const RGPassParameters& passParams, std::function<void(RHICommandBuffer&)> fnExecute)
{
    _passes.emplace_back(passName, passParams, fnExecute);
}

void RenderGraphBuilder::Setup(RHICommandBuffer* cmdBuffer)
{
    _currentCmdBuffer = cmdBuffer;
    _frameIndex       = (_frameIndex + 1) % s_maxFramesInFlight;
}

void RenderGraphBuilder::Compile()
{
    for (auto& pass : _passes)
    {
        HS_ASSERT(pass.IsCompiled() == false, "Pass %s is already compiled!", pass._name);

        // TODO: RGPassParameters를 기반으로 의존성 분석하기
        // Parameters안의 Descriptor들을 기반으로 RDGResource를 만들고, RGPass와 RGResource를 연결하기
        // 이 때 RDGResource를 만들되 실제 RHIHandle과 연결은 하지 않고(null로 유지), RGPass의 upstream/downstream 관계만 설정하기
        // 근데 여기서 사용 리소스를 최소화하려면 어떤 체크들을 해야 할까..
        // 

        pass._isCompiled = true;

    }
}

void RenderGraphBuilder::Execute()
{
    for (auto& pass : _passes)
    {
        traverse(&pass);
    }
}

void RenderGraphBuilder::Reset()
{
    _passes.clear();
    _currentCmdBuffer = nullptr;
}

void RenderGraphBuilder::addDependency(RGTextureDescriptor& desc, RGPass* pass)
{
}

void RenderGraphBuilder::addDependency(RGBufferDescriptor& desc, RGPass* pass)
{
}

void RenderGraphBuilder::traverse(RGPass* pass)
{
    HS_ASSERT(pass->IsCompiled(), "Pass %s is not compiled yet!", pass->_name);

    if (pass->IsCulled() || pass->IsExecuted())
    {
        return;
    }

    for (auto* upstream : pass->_upstreams)
    {
        traverse(upstream);
    }

    pass->Execute(*_currentCmdBuffer);
}

HS_NS_END