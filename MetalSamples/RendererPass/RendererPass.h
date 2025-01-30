#ifndef __RENDERER_PASS_H__
#define __RENDERER_PASS_H__
//
//  RendererPass.h
//  MetalSamples
//
//  Created by Yongsik Im on 1/30/25.
//

#include <typeinfo>

#import <Metal/Metal.h>
#import <simd/simd.h>

class HSRenderer;

class HSRendererPass
{
public:
    HSRendererPass(const char* name, HSRenderer* renderer, uint32_t renderingOrder);
    virtual ~HSRendererPass() = default;

    virtual void OnInit() = 0;

    virtual void OnBeforeRendering(uint32_t submitIndex) = 0;

    virtual void Configure(id<MTLTexture> renderTarget) = 0;

    virtual void Execute(id<MTLCommandBuffer> cmdBuffer) = 0;

    virtual void OnAfterRendering() = 0;

    virtual void Clear() {}

    bool IsExecutable() { return _isExecutable; }

    const char* name;

    uint32_t renderingOrder;

protected:
    HSRenderer* _renderer;
    bool _isExecutable = true;
    size_t _submitIndex;
};

#endif
