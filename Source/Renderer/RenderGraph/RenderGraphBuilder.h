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

    RGPass AddPass(const char* passName, std::function<void()> fnSetup, std::function<void(RHICommandBuffer&)> fnExecute);

    void AddDependency(RGPass& after, RGPass& before);

    void Setup(RHICommandBuffer* cmdBuffer);
    void Compile();
    void Execute();
    void Reset();

private:
    constexpr static uint8 s_maxFramesInFlight = 2;

    RHICommandBuffer* _currentCmdBuffer = nullptr;

    std::unordered_map<const char*, RGTexture*> _inFlightTextures;
    std::unordered_map<const char*, RGBuffer*> _inFlightBuffers;

    std::vector<RGTexture*> _freeTextures[s_maxFramesInFlight];
    std::vector<RGBuffer*> _freeBuffers[s_maxFramesInFlight];

    std::vector<RGPass> _passes;
    std::vector<RGPass&> _sortedPass;
    
    std::unordered_map<RGPass*, std::vector<RGPass*>> _passDependencyMap;

    uint8 _frameIndex = static_cast<uint8>(-1);
};

HS_NS_END

#endif
