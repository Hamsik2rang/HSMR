#include "Engine/RendererPass/Forward/ForwardPass.h"

HS_NS_BEGIN

RHI_RESOURCE_BEGIN(ForwardPass)

RHI_RESOURCE_END(ForwardPass)

ForwardPass::ForwardPass(const char* name, Renderer* renderer, ERenderingOrder renderingOrder)
: RendererPass(name, renderer, renderingOrder)
{
}


HS_NS_END
