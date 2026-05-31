#ifndef __HS_RENDER_PASS_H__
#define __HS_RENDER_PASS_H__

#include "Renderer/RenderGraph.h"

class IRenderPass
{
public:
    ERGPassFlag GetPassFlag() = 0;
    
    void SetPassFlag(ERGPassFlag passFlag) = 0;

    void Setup(RenderGraphBuilder& builder) = 0;
    
    void SetRenderTarget(RenderTarget& renderTarget) = 0;
};

#endif
