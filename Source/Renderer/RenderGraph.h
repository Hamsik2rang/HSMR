#ifndef __HS_RENDER_GRAPH_H__
#define __HS_RENDER_GRAPH_H__

#include "Precompile.h"

HS_NS_BEGIN

class RenderGraph
{
public:
    class Builder
    {
    public:
    };
    Builder& GetBuilder() { return _builder; }

private:
    Builder _builder;
};

HS_NS_END

#endif