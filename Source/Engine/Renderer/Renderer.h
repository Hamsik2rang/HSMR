//
//  Renderer.h
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//
#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Precompile.h"

#include "Engine/Renderer/RenderTarget.h"
#include "Engine/Renderer/RenderDefinition.h"
#include "Engine/RHI/RHIDefinition.h"
#include "Engine/RHI/RHIContext.h"

#include <vector>
#include <unordered_map>

#include <SDL3/SDL.h>

HS_NS_BEGIN

class EngineContext;
class RendererPass;
class Scene;
class Swapchain;
class Framebuffer;

struct NativeWindowHandle;

class Renderer
{
public:
    class RHIHandleCache
    {
        friend Renderer;

    public:
        RHIHandleCache(Renderer* renderer);
        ~RHIHandleCache();

        RenderPass*       GetRenderPass(const RenderPassInfo& info);
        Framebuffer*      GetFramebuffer(RenderPass* renderPass, RenderTarget* renderTarget);
        GraphicsPipeline* GetGraphicsPipeline(const GraphicsPipelineInfo& info);

    private:
        Renderer* _renderer;

        std::unordered_map<uint32, RenderPass*>       _renderPassCache;
        std::unordered_map<uint32, Framebuffer*>      _framebufferCache;
        std::unordered_map<uint32, GraphicsPipeline*> _gPipelineCache;
    };

    Renderer(RHIContext* rhiContext);
    virtual ~Renderer();

    virtual bool Initialize();

    virtual void NextFrame(Swapchain* swapchain);

    virtual void Render(const RenderParameter& param, RenderTarget* renderTexture);

    void AddPass(RendererPass* pass)
    {
        _rendererPasses.push_back(pass);
        _isPassListSorted = false;
    }

    virtual void Shutdown();

    HS_FORCEINLINE RHIContext* GetRHIContext() { return _rhiContext; }

    HS_FORCEINLINE uint32 GetCurrentFrameIndex() { return _frameIndex; }

    virtual RenderTargetInfo GetBareboneRenderTargetInfo() = 0;

    HS_FORCEINLINE RHIHandleCache* GetHandleCache() const { return _rhiHandleCache; }

protected:
    RHIContext*     _rhiContext;
    RHIHandleCache* _rhiHandleCache;
    CommandBuffer*  _curCommandBuffer; // TODO: Multi-CommandBuffer 구현 필요

    std::vector<RendererPass*> _rendererPasses;
    uint32                     _frameIndex       = 0;
    bool                       _isInitialized    = false;
    bool                       _isPassListSorted = true;

    RenderTarget* _currentRenderTarget;
};

HS_NS_END

#endif
