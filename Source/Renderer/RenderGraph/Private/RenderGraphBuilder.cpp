#include "Renderer/RenderGraph/RenderGraphBuilder.h"

HS_NS_BEGIN

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

RGPass RenderGraphBuilder::AddPass(const char* passName, std::function<void()> fnSetup, std::function<void(RHICommandBuffer&)> fnExecute)
{
    RGPass pass{passName, fnSetup, fnExecute};

    _passes.push_back(pass); // TODO: Name 중복 해결 필요

    return pass;
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
        // 직접 걸어준 의존성은 컴파일에서 신경 안써도 됨
//        if (_passDependencyMap.find(&pass) != _passDependencyMap.end())
//        {
//            
//        }
        pass.
    }
}

void RenderGraphBuilder::Execute()
{
    for (auto& pass : _sortedPass)
    {
        pass.Execute(_currentCmdBuffer);
    }
}

void RenderGraphBuilder::Reset()
{
    _currentCmdBuffer = nullptr;
}

HS_NS_END
