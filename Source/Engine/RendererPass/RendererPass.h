#ifndef __RENDERER_PASS_H__
#define __RENDERER_PASS_H__
//
//  RendererPass.h
//  MetalSamples
//
//  Created by Yongsik Im on 1/30/25.
//

#include "Precompile.h"

#include <typeinfo>

#import <Metal/Metal.h>
#import <simd/simd.h>

#include "Engine/ThirdParty/ImGui/imgui.h"

HS_NS_BEGIN

class Renderer;

class RendererPass
{
public:
    RendererPass(const char* name, Renderer* renderer, uint32 renderingOrder);
    virtual ~RendererPass() = default;

    virtual void Initialize() = 0;
    
    virtual void Finalize() = 0;

    virtual void OnBeforeRendering(uint32_t submitIndex) = 0;

    virtual void Configure(id<MTLTexture> renderTarget) = 0;

    virtual void Execute(id<MTLRenderCommandEncoder> renderEncoder) = 0;

    virtual void OnAfterRendering() = 0;
    
    virtual void Clear() {}

    bool IsExecutable() { return _isExecutable; }

    const char* name;

    uint32_t renderingOrder;

protected:
    Renderer* _renderer;
    bool _isExecutable = true;
    size_t _submitIndex;
};

HS_NS_END

#endif
