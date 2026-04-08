//
//  Renderer.h
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
#include "Renderer/RenderResourceManager.h"

#include "Renderer/RenderGraph.h"

#include <vector>
#include <unordered_map>

namespace hs
{
/*#include "Renderer/RenderPass/RenderPass.h"*/ class RenderPass;
/*#include "RHI/RenderHandle.h"*/ class RHIFramebuffer;
/*#include "RHI/Swapchain.h"*/ class Swapchain;
/*#include "Platform/NativeWindow.h"*/ struct NativeWindow;
/*#include "Renderer/ShaderLibrary.h"*/ class ShaderLibrary;
} // namespace hs

HS_NS_BEGIN

class HS_RENDERER_API Renderer
{
public:
    class RHIHandleCache
    {
        friend Renderer;

    public:
        RHIHandleCache(Renderer* renderer);
        ~RHIHandleCache();

        RHIRenderPass* GetRenderPass(const RenderPassInfo& info);
        RHIFramebuffer* GetFramebuffer(RHIRenderPass* renderPass, RenderTarget* renderTarget);
        RHIGraphicsPipeline* GetGraphicsPipeline(const GraphicsPipelineInfo& info);

    private:
        Renderer* _renderer;

        std::unordered_map<size_t, RHIRenderPass*> _renderPassCache;
        std::unordered_map<size_t, RHIFramebuffer*> _framebufferCache;
        std::unordered_map<size_t, RHIGraphicsPipeline*> _gPipelineCache;
    };

    Renderer(RHIContext* rhiContext);
    virtual ~Renderer();

    virtual bool Initialize();

    virtual void NextFrame(Swapchain* swapchain);

    virtual void Render(
        Scene* scene,
        RenderTarget* renderTarget
    );

    virtual void Render(
        const RenderSceneSnapshot& snapshot,
        RenderTarget* renderTarget
    );

    virtual void AddPass(RenderPass* pass)
    {
        _rendererPasses.push_back(pass);
        _isPassListSorted = false;
    }

    virtual void Shutdown();

    HS_FORCEINLINE RHIContext* GetRHIContext() { return _rhiContext; }

    HS_FORCEINLINE uint32 GetCurrentFrameIndex() { return frameIndex; }

    HS_FORCEINLINE RHIHandleCache* GetHandleCache() const { return _rhiHandleCache; }

    HS_FORCEINLINE RenderResourceManager* GetResourceManager() const { return _resourceManager; }

    HS_FORCEINLINE ShaderLibrary* GetShaderLibrary() const { return _shaderLibrary; }

protected:
    RHIContext* _rhiContext;
    RHIHandleCache* _rhiHandleCache;
    RHICommandBuffer* _curCommandBuffer; // TODO: Multi-CommandBuffer 구현 필요
    RenderResourceManager* _resourceManager = nullptr;
    ShaderLibrary* _shaderLibrary = nullptr;

    std::vector<RenderPass*> _rendererPasses;
    uint32 frameIndex      = 0;
    bool _isInitialized    = false;
    bool _isPassListSorted = true;

    RenderTarget* _currentRenderTarget;

    RenderGraphBuilder _graphBuilder;
};

HS_NS_END

#endif
