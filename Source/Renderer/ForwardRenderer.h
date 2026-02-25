#ifndef __HS_FORWARD_RENDERER_H__
#define __HS_FORWARD_RENDERER_H__

#include "Precompile.h"

#include "Renderer/Renderer.h"

HS_NS_BEGIN

class HS_RENDERER_API ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(RHIContext* rhiContext);
    ~ForwardRenderer() override;

    //...

private:
};

HS_NS_END

#endif /* __HS_FORWARD_RENDERER_H__ */
