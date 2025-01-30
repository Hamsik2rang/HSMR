//
//  RendererPass.m
//  MetalSamples
//
//  Created by Yongsik Im on 1/30/25.
//

#include "RendererPass.h"
#include "Renderer.h"

#import <Metal/Metal.h>

HSRendererPass::HSRendererPass(const char* name, HSRenderer* renderer, uint32_t renderingOrder)
    : name(name)
    , _renderer(renderer)
    , renderingOrder(renderingOrder)
{
}
