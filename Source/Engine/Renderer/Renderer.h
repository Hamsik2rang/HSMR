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
#include "Engine/RHI/RHIDefinition.h"
#include "Engine/RHI/RHIContext.h"

#include <vector>

#include <SDL3/SDL.h>

HS_NS_BEGIN

//TODO: RHIContext 분리 필요

class EngineContext;
class RendererPass;
class Scene;
class Swapchain;
class Framebuffer;

struct NativeWindowHandle;

class Renderer
{
public:
    Renderer(RHIContext* rhiContext);
    ~Renderer();

    virtual bool Initialize();

    virtual void NextFrame(Swapchain* swapchain);

    virtual void Render(const RenderParameter& param, RenderTarget* renderTexture);

    virtual void Present(Swapchain* swapchain);

    void AddPass(RendererPass* pass)
    {
        _rendererPasses.push_back(pass);
        _isPassListSorted = false;
    }
    
    virtual void Shutdown();
    
    HS_FORCEINLINE RHIContext* GetRHIContext() { return _rhiContext; }
    
    HS_FORCEINLINE uint32 GetCurrentFrameIndex() { return _frameIndex; }
    
protected:
    RHIContext* _rhiContext;
    
    CommandBuffer* _commandBuffer[3]; // TODO: Multi-CommandBuffer 구현 필요
    
    std::vector<RendererPass*> _rendererPasses;
    uint32 _frameIndex = 0;
    bool _isInitialized = false;
    bool _isPassListSorted = true;
    
    Swapchain* _swapchain = nullptr;
    
    RenderTarget* _currentRenderTarget;
};

HS_NS_END

#endif
