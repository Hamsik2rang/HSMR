//
//  RendererPass.m
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//

#include "Core/RendererPass/RendererPass.h"
#include "Core/Renderer/Renderer.h"

HS_NS_BEGIN
RendererPass::RendererPass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder)
    : name(name)
    , _renderer(renderer)
    , renderingOrder(renderingOrder)
{
}

HS_NS_END
