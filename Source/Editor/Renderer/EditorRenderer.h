#ifndef __HS_EDITOR_RENDERER_EDITOR_RENDERER_H__
#define __HS_EDITOR_RENDERER_EDITOR_RENDERER_H__

#include "Precompile.h"

#include "Renderer/RendererDefinition.h"
#include "Renderer/RenderGraph/RenderGraphBuilder.h"
#include "Editor/Renderer/RenderPass/EditorDebugPass.h"
#include "Editor/Renderer/RenderPass/EditorGridPass.h"
#include "Editor/Renderer/RenderPass/EditorIconPass.h"

namespace hs
{
class RHIContext;
class RHICommandBuffer;
class RenderResourceManager;
class RenderTarget;
class ShaderLibrary;
} // namespace hs

HS_NS_BEGIN

class HS_EDITOR_API EditorRenderer
{
public:
    explicit EditorRenderer(RHIContext* rhiContext);
    ~EditorRenderer();

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

    Scoped<EditorGridPass> _gridPass;
    Scoped<EditorDebugPass> _debugPass;
    Scoped<EditorIconPass> _iconPass;
    bool _isInitialized = false;
};

HS_NS_END

#endif /* __HS_EDITOR_RENDERER_EDITOR_RENDERER_H__ */
