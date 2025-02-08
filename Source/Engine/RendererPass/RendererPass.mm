//
//  RendererPass.m
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//

#include "Engine/RendererPass/RendererPass.h"
#include "Engine/Renderer/Renderer.h"

#import <Metal/Metal.h>


HS_NS_BEGIN

RendererPass::RendererPass(const char* name, Renderer* renderer, uint32_t renderingOrder)
    : name(name)
    , _renderer(renderer)
    , renderingOrder(renderingOrder)
{
}


HS_NS_END
