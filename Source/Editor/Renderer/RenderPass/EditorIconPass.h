#ifndef __HS_EDITOR_RENDERER_RENDER_PASS_EDITOR_ICON_PASS_H__
#define __HS_EDITOR_RENDERER_RENDER_PASS_EDITOR_ICON_PASS_H__

#include "Precompile.h"
#include "RHI/RHIDefinition.h"
#include "Renderer/RendererDefinition.h"

#include <unordered_map>
#include <vector>

namespace hs
{
class RHIContext;
class RHIShader;
class RHIBuffer;
class RHIResourceLayout;
class RHIResourceSet;
class RHIGraphicsPipeline;
class ShaderLibrary;
class RenderResourceManager;
class Image;
struct ImageResource;
} // namespace hs

HS_NS_BEGIN

class HS_EDITOR_API EditorIconPass
{
public:
    EditorIconPass() = default;
    ~EditorIconPass();

    bool Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext);
    void Shutdown();

    bool Prepare(RenderResourceManager& resourceManager,
                 const RenderSceneSnapshot& snapshot,
                 const RenderViewSnapshot& viewSnapshot);

    RHIGraphicsPipeline* GetOrCreatePipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                             RHIBuffer* perViewBuffer);

    HS_FORCEINLINE bool IsInitialized() const { return _isInitialized; }
    HS_FORCEINLINE bool HasDrawData() const
    {
        return _vertexBuffer != nullptr && _instanceBuffer != nullptr && _instanceCount > 0;
    }
    HS_FORCEINLINE RHIBuffer* GetVertexBuffer() const { return _vertexBuffer; }
    HS_FORCEINLINE RHIBuffer* GetInstanceBuffer() const { return _instanceBuffer; }
    HS_FORCEINLINE uint32 GetVertexCount() const { return _vertexCount; }
    HS_FORCEINLINE uint32 GetInstanceCount() const { return _instanceCount; }
    HS_FORCEINLINE RHIResourceSet* GetResourceSet() const { return _resourceSet; }

private:
    struct IconVertex
    {
        glm::vec2 localPosition;
        glm::vec2 uv;
    };

    struct IconInstance
    {
        glm::vec4 worldPositionAndSize;
        glm::vec4 tintAndAlpha;
        glm::vec4 iconMeta;
    };

    enum class EIconType : uint32
    {
        Camera = 0,
        Light = 1,
    };

    bool ensureIconResources(RenderResourceManager& resourceManager);
    void rebuildResourceBindings(RHIBuffer* perViewBuffer);
    void resetPipelines();
    void addCameraIcon(const DebugCameraSnapshot& camera, const RenderViewSnapshot& viewSnapshot);
    void addLightIcon(const DebugLightSnapshot& light, const RenderViewSnapshot& viewSnapshot);
    void addIconInstance(const glm::vec3& worldPosition,
                         const RenderViewSnapshot& viewSnapshot,
                         EIconType iconType,
                         const glm::vec3& tint,
                         float alpha);

    RHIContext* _rhiContext = nullptr;
    RHIShader* _vertexShader = nullptr;
    RHIShader* _fragmentShader = nullptr;
    RHIResourceLayout* _resourceLayout = nullptr;
    RHIResourceSet* _resourceSet = nullptr;
    RHIBuffer* _perViewBuffer = nullptr;
    RHIBuffer* _vertexBuffer = nullptr;
    RHIBuffer* _instanceBuffer = nullptr;

    Scoped<Image> _cameraIconImage;
    Scoped<Image> _lightIconImage;
    ImageResource* _cameraIconResource = nullptr;
    ImageResource* _lightIconResource = nullptr;

    std::vector<IconInstance> _instances;
    std::unordered_map<size_t, RHIGraphicsPipeline*> _pipelineCache;
    uint32 _instanceCapacity = 0;
    uint32 _instanceCount = 0;
    uint32 _vertexCount = 0;
    bool _isInitialized = false;
};

HS_NS_END

#endif /* __HS_EDITOR_RENDERER_RENDER_PASS_EDITOR_ICON_PASS_H__ */
