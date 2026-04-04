#ifndef __HS_FORWARD_RENDERER_H__
#define __HS_FORWARD_RENDERER_H__

#include "Precompile.h"

#include "Renderer/Renderer.h"
#include "Renderer/RenderPass/ForwardGridPass.h"

HS_NS_BEGIN

class HS_RENDERER_API ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(RHIContext* rhiContext);
    ~ForwardRenderer() override;

    void Render(Scene* scene, RenderTarget* renderTarget) override;
    void Shutdown() override;

    //...

private:
    Scoped<ForwardGridPass> _gridPass;
};

HS_NS_END

#endif /* __HS_FORWARD_RENDERER_H__ */
