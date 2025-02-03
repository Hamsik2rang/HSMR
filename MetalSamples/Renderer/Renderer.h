//
//  Renderer.h
//  MetalSamples
//
//  Created by Yongsik Im on 1/29/25.
//
#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <vector>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#include <SDL3/SDL.h>

class HSScene;
class HSRendererPass;

extern std::string hs_get_resource_path(const char* filePath);

class HSRenderer
{
public:
    HSRenderer(SDL_Window* window);
    ~HSRenderer();

    bool Init();

    void NextFrame();

    void Render(HSScene* scene);

    void Present();

    void AddPass(HSRendererPass* pass)
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

    std::vector<HSRendererPass*> _rendererPasses;
    uint32_t _submitIndex = 0;
    SDL_Window* _window;
    void* _view;
    CAMetalLayer* _layer;
    bool _isInitialized = false;
    bool _isPassListSorted = true;
};

#endif
