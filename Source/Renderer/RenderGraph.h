#ifndef __HS_RENDER_GRAPH_H__
#define __HS_RENDER_GRAPH_H__

#include "Precompile.h"

#include "RHI/CommandHandle.h"
#include "Renderer/RenderDefinition.h"

HS_NS_BEGIN

class RenderGraphBuilder
{
public:

    RenderGraphBuilder(RHICommandBuffer& cmdBuffer);
    ~RenderGraphBuilder();
    
    RHITexture* AcquireTexture();
private:

};

HS_NS_END

#endif
