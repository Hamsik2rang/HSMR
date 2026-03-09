#include "Renderer/ForwardRenderer_RG.h"

HS_NS_BEGIN

ForwardRendererRG::ForwardRendererRG(RHIContext* rhiContext)
    : _rhiContext(rhiContext)
{
}

void ForwardRendererRG::Render(Scene* scene, RHICommandBuffer& cmdBuffer, RenderTarget* renderTarget)
{
    _graphBuilder.AddPass("PreDepth", [&]() {

    },
                          [&](RHICommandBuffer& cmdBuffer) {

                          });
}

HS_NS_END
