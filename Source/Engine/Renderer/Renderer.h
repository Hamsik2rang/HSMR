//
//  Renderer.h
//  HSMR
//
//  Created by Yongsik Im on 1/29/25.
//
#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Precompile.h"
#include "Engine/Renderer/RenderDefinition.h"

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
    Renderer();
    ~Renderer();

    virtual bool Initialize(const NativeWindowHandle* nativeHandle);

    virtual void NextFrame(Swapchain* swapchain);

    virtual void Render(const RenderParameter& param, Framebuffer* renderTarget);

    virtual void Present(Swapchain* swapchain);

    void AddPass(RendererPass* pass)
    {
        _rendererPasses.push_back(pass);
        _isPassListSorted = false;
    }
    
    virtual void* GetDevice() { return _rhiDevice; }
    
    virtual void Shutdown();
    
    static constexpr uint32_t MAX_SUBMIT_INDEX = 3;
    
protected:
    void renderDockingPanel();
//    id<MTLDevice> _device;
//    id<MTLCommandQueue> _commandQueue;
//    id<MTLCommandBuffer> _commandBuffer;
//    id<MTLRenderCommandEncoder> _renderEncoder;
//
//    id<MTLTexture> _renderTarget[MAX_SUBMIT_INDEX];
//    id<CAMetalDrawable> _currentDrawable;
//    MTLRenderPassDescriptor* _renderPassDescriptor;
    void* _window;
    void* _view;
    void* _layer;
    
    void* _rhiDevice;
    void* _commandQueue;
    void* _curCommandBuffer;
    void* _curCommandEncoder;
    
    std::vector<RendererPass*> _rendererPasses;
    uint32_t _submitIndex = 0;
    bool _isInitialized = false;
    bool _isPassListSorted = true;
    
    static const size_t MAX_FRAME_COUNT;
    uint32 _frameCount;
};

HS_NS_END

#endif
