#ifndef __HS_FORWARD_RENDERER_H__
#define __HS_FORWARD_RENDERER_H__

#include "Precompile.h"

#include "Engine/Renderer/Renderer.h"

HS_NS_BEGIN

class ForwardRenderer : public Renderer
{
public:
    ForwardRenderer(RHIContext* rhiContext);
    ~ForwardRenderer() override;
    
    //...
    
    RenderTargetInfo GetBareboneRenderTargetInfo() override { return s_bareboneRenderTargetInfo; }

private:
    const static RenderTargetInfo s_bareboneRenderTargetInfo;
};

HS_NS_END

#endif /* __HS_FORWARD_RENDERER_H__ */
