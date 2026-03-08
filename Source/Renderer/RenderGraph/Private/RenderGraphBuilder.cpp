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

void RenderGraphBuilder::AddPass(const char* passName, std::function<void()>& fnSetup, std::function<void(RHICommandBuffer&)>& fnExecute)
{
}

void RenderGraphBuilder::Setup(RHICommandBuffer* cmdBuffer)
{
    _currentCmdBuffer = cmdBuffer;
    _frameIndex       = (_frameIndex + 1) % s_maxFramesInFlight;
}

void RenderGraphBuilder::Compile()
{

}

void RenderGraphBuilder::Execute()
{
}

void RenderGraphBuilder::Reset()
{
    _currentCmdBuffer = nullptr;
}




HS_NS_END