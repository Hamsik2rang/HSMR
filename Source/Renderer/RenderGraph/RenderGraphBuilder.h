#ifndef __HS_RENDER_GRAPH_BUILDER_H__
#define __HS_RENDER_GRAPH_BUILDER_H__

#include "Precompile.h"

#include "RHI/CommandHandle.h"
#include "Renderer/RenderDefinition.h"

#include "Renderer/RenderGraph/RenderGraphResource.h"

#include <functional>

HS_NS_BEGIN 

class RenderGraphBuilder
{
public:
    RenderGraphBuilder();
    ~RenderGraphBuilder();

    RGTexture* RegisterExternalTexture(RHITexture* texture);
    void UnregisterExternalTexture(RHITexture* texture);
    RGTexture* AcquireTexture(const RGTextureDescriptor& desc);
    RGTexture* FindTexture(RHITexture* texture) const; 

    RGBuffer* RegisterExternalBuffer(RHIBuffer* buffer);
    void UnregisterExternalBuffer(RHIBuffer* buffer);
    RGBuffer* AcquireBuffer(const RGBufferDescriptor& desc);
    RGBuffer* FindBuffer(RHIBuffer* buffer) const;

    void AddPass(const char* passName, std::function<void()>& fnSetup, std::function < void(RHICommandBuffer&)>& fnExecute);

    void Setup(RHICommandBuffer* cmdBuffer);
    void Compile();
    void Execute();
    void Reset();

private:
    RHICommandBuffer* _currentCmdBuffer = nullptr;

    uint8 _frameIndex = static_cast<uint8>(-1);
    constexpr static uint8 s_maxFramesInFlight = 2;
};

HS_NS_END 

#endif