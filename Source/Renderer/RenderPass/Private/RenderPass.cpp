//
//  RenderPass.m
//  HSMR
//
//  Created by Yongsik Im on 1/30/25.
//

#include "RenderPass/RenderPass.h"
#include "Renderer/RenderPath.h"

HS_NS_BEGIN
RenderPass::RenderPass(const char* name, RenderPath* renderer, ERenderingOrder renderingOrder)
    : name(name)
    , _renderer(renderer)
    , renderingOrder(renderingOrder)
{
}

HS_NS_END
