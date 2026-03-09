#ifndef __HS_FORWARD_RENDERER_RG_H__
#define __HS_FORWARD_RENDERER_RG_H__

#include "Precompile.h"

#include "Renderer/RenderGraph.h"
#include "Renderer/Renderer.h"

HS_NS_BEGIN

class ForwardRendererRG
{
public:
    ForwardRendererRG(RHIContext* rhiContext);
    ~ForwardRendererRG() = default;
    void Render(Scene* scene, RHICommandBuffer& cmdBuffer, RenderTarget* renderTarget);

private:

    RHIContext* _rhiContext;
    RenderGraphBuilder _graphBuilder;
};

HS_NS_END

#endif
