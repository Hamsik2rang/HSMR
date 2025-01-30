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
#include "ViewController.h"

class HSRendererPass;

class HSRenderer
{
public:
    HSRenderer(id<MTLDevice> device);
    ~HSRenderer();

    bool Init(HSView* view);

    void NextFrame();

    void Render();

    void Present(id<MTLDrawable> currentDrawable);

    id<MTLDevice> GetDevice() { return _device; }

    void AddPass(HSRendererPass* pass) { _rendererPasses.push_back(pass); }

    HSView* GetView();
    static constexpr uint32_t MAX_SUBMIT_INDEX = 3;

private:
    HSView* _Nonnull _view;

    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLCommandBuffer> _commandBuffer;
    id<MTLTexture> _renderTarget[MAX_SUBMIT_INDEX];

    std::vector<HSRendererPass*> _rendererPasses;
    uint32_t _submitIndex = 0;
};

#endif
