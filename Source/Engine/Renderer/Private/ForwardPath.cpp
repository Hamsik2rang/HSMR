#include "Renderer/ForwardPath.h"
#include "RHI/RHIContext.h"
HS_NS_BEGIN

ForwardRenderer::ForwardRenderer(RHIContext* rhiContext)
    : RenderPath(rhiContext)
{
}

ForwardRenderer::~ForwardRenderer()
{
}

HS_NS_END
