//
//  RenderPath.h
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//
#ifndef __HS_RENDERER_H__
#define __HS_RENDERER_H__

#include "Precompile.h"

#include "Renderer/RenderTarget.h"
#include "Renderer/RendererDefinition.h"
#include "RHI/RHIDefinition.h"
#include "RHI/RHIContext.h"

#include <vector>
#include <unordered_map>

namespace HS { class EngineContext; }
namespace HS { class RenderPass; }
namespace HS { class Scene; }
namespace HS { class Swapchain; }
namespace HS { class RHIFramebuffer; }
namespace HS { struct NativeWindow; }

HS_NS_BEGIN

class RenderPath
{
public:
    class RHIHandleCache
    {
        friend RenderPath;

    public:
        RHIHandleCache(RenderPath* renderer);
        ~RHIHandleCache();

        RHIRenderPass*       GetRenderPass(const RenderPassInfo& info);
        RHIFramebuffer*      GetFramebuffer(RHIRenderPass* renderPass, RenderTarget* renderTarget);
        RHIGraphicsPipeline* GetGraphicsPipeline(const GraphicsPipelineInfo& info);

    private:
        RenderPath* _renderer;

        std::unordered_map<uint32, RHIRenderPass*>       _renderPassCache;
        std::unordered_map<uint32, RHIFramebuffer*>      _framebufferCache;
        std::unordered_map<uint32, RHIGraphicsPipeline*> _gPipelineCache;
    };

    RenderPath(RHIContext* rhiContext);
    virtual ~RenderPath();

    virtual bool Initialize();

    virtual void NextFrame(Swapchain* swapchain);

    virtual void Render(const RenderParameter& param, RenderTarget* renderTexture);

    virtual void AddPass(RenderPass* pass)
    {
        _rendererPasses.push_back(pass);
        _isPassListSorted = false;
    }

    virtual void Shutdown();

    HS_FORCEINLINE RHIContext* GetRHIContext() { return _rhiContext; }

    HS_FORCEINLINE uint32 GetCurrentFrameIndex() { return frameIndex; }

    virtual RenderTargetInfo GetBareboneRenderTargetInfo() = 0;

    HS_FORCEINLINE RHIHandleCache* GetHandleCache() const { return _rhiHandleCache; }

protected:
    RHIContext*     _rhiContext;
    RHIHandleCache* _rhiHandleCache;
    RHICommandBuffer*  _curCommandBuffer; // TODO: Multi-CommandBuffer 구현 필요

    std::vector<RenderPass*> _rendererPasses;
    uint32                     frameIndex       = 0;
    bool                       _isInitialized    = false;
    bool                       _isPassListSorted = true;

    RenderTarget* _currentRenderTarget;
};

HS_NS_END

#endif
