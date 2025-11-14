#include "Engine/Renderer/RenderPass/ForwardRenderPass.h"

HS_NS_BEGIN


ForwardRenderPass::ForwardRenderPass(const char* name, RenderPath* renderer, ERenderingOrder renderingOrder)
: RenderPass(name, renderer, renderingOrder)
{
}

HS_NS_END
