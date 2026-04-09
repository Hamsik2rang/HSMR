#ifndef __HS_FORWARD_OVERLAY_RENDERER_H__
#define __HS_FORWARD_OVERLAY_RENDERER_H__

#include "Precompile.h"

#include "Renderer/RendererDefinition.h"
#include "Renderer/RenderGraph/RenderGraphBuilder.h"
#include "Renderer/RenderPass/ForwardDebugPass.h"
#include "Renderer/RenderPass/ForwardGridPass.h"

namespace hs
{
class RHIContext;
class RHICommandBuffer;
class RenderResourceManager;
class RenderTarget;
class ShaderLibrary;
} // namespace hs

HS_NS_BEGIN

class HS_RENDERER_API ForwardOverlayRenderer
{
public:
    explicit ForwardOverlayRenderer(RHIContext* rhiContext);
    ~ForwardOverlayRenderer();

    bool Initialize(ShaderLibrary* shaderLibrary);
    void Shutdown();

    void Render(
        RHICommandBuffer& commandBuffer,
        RenderResourceManager& resourceManager,
        const RenderSceneSnapshot& snapshot,
        const RenderViewSnapshot& viewSnapshot,
        RenderTarget* renderTarget,
        const RenderOptions& options);

private:
    RHIContext* _rhiContext = nullptr;
    ShaderLibrary* _shaderLibrary = nullptr;
    RenderGraphBuilder _graphBuilder;

    Scoped<ForwardGridPass> _gridPass;
    Scoped<ForwardDebugPass> _debugPass;
    bool _isInitialized = false;
};

HS_NS_END

#endif /* __HS_FORWARD_OVERLAY_RENDERER_H__ */
