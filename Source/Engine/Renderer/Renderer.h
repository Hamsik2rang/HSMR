//
//  Renderer.h
//  MetalSamples
//
//  Created by Yongsik Im on 1/29/25.
//
#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Precompile.h"

#include <vector>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <SDL3/SDL.h>

HS_NS_BEGIN

class Scene;
class RendererPass;

extern std::string hs_get_resource_path(const char* filePath);

class Renderer
{
public:
    Renderer(SDL_Window* window);
    ~Renderer();

    bool Init();

    void NextFrame();

    void Render(Scene* scene);

    void Present();

    void AddPass(RendererPass* pass)
    {
        _rendererPasses.push_back(pass);
        _isPassListSorted = false;
    }

    void* GetView() { return _view; }
    
    id<MTLDevice> GetDevice() { return _device; }
    
    void Shutdown();

    void SetFont(void* font);

    static constexpr uint32_t MAX_SUBMIT_INDEX = 3;

private:
    void renderDockingPanel();

    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLCommandBuffer> _commandBuffer;
    id<MTLRenderCommandEncoder> _renderEncoder;

    id<MTLTexture> _renderTarget[MAX_SUBMIT_INDEX];
    id<CAMetalDrawable> _currentDrawable;
    MTLRenderPassDescriptor* _renderPassDescriptor;

    std::vector<RendererPass*> _rendererPasses;
    uint32_t _submitIndex = 0;
    SDL_Window* _window;
    void* _view;
    CAMetalLayer* _layer;
    bool _isInitialized = false;
    bool _isPassListSorted = true;
};

HS_NS_END

#endif
