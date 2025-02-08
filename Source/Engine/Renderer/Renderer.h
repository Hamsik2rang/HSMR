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

struct NativeWindowHandle;

class Renderer
{
public:
    Renderer();
    ~Renderer();

    virtual bool Initialize(const NativeWindowHandle* nativeHandle);

    virtual void NextFrame();

    virtual void Render(const RenderParameter& param, RenderTexture* renderTarget);

    virtual void Present();

    void AddPass(RendererPass* pass)
    {
        _rendererPasses.push_back(pass);
        _isPassListSorted = false;
    }
    
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
    
    std::vector<RendererPass*> _rendererPasses;
    uint32_t _submitIndex = 0;
    bool _isInitialized = false;
    bool _isPassListSorted = true;
};

HS_NS_END

#endif
