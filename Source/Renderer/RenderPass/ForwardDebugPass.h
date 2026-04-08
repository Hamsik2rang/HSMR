//
//  ForwardDebugPass.h
//  Renderer
//
//  Scene debug line overlay pass.
//
#ifndef __HS_RENDERER_RENDER_PASS_FORWARD_DEBUG_PASS_H__
#define __HS_RENDERER_RENDER_PASS_FORWARD_DEBUG_PASS_H__

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
} // namespace hs

HS_NS_BEGIN

class HS_RENDERER_API ForwardDebugPass
{
public:
    ForwardDebugPass() = default;
    ~ForwardDebugPass();

    bool Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext);
    void Shutdown();

    bool Prepare(const RenderSceneSnapshot& snapshot);

    RHIGraphicsPipeline* GetOrCreatePipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                             RHIBuffer* perViewBuffer);

    HS_FORCEINLINE bool IsInitialized() const { return _isInitialized; }

    HS_FORCEINLINE bool HasDrawData() const { return _vertexCount > 0 && _vertexBuffer != nullptr; }

    HS_FORCEINLINE RHIBuffer* GetVertexBuffer() const { return _vertexBuffer; }

    HS_FORCEINLINE uint32 GetVertexCount() const { return _vertexCount; }

    HS_FORCEINLINE RHIResourceSet* GetResourceSet() const { return _resourceSet; }

private:
    struct DebugLineVertex
    {
        glm::vec3 position;
        glm::vec4 color;
    };

    void rebuildResourceBindings(RHIBuffer* perViewBuffer);
    void resetPipelines();
    void addLine(const glm::vec3& from, const glm::vec3& to, const glm::vec4& color);
    void addCamera(const DebugCameraSnapshot& camera);
    void addLight(const DebugLightSnapshot& light);
    void addCircle(const glm::vec3& center,
                   const glm::vec3& axisA,
                   const glm::vec3& axisB,
                   float radius,
                   const glm::vec4& color,
                   uint32 segmentCount);

    RHIContext*        _rhiContext     = nullptr;
    RHIShader*         _vertexShader   = nullptr;
    RHIShader*         _fragmentShader = nullptr;
    RHIResourceLayout* _resourceLayout = nullptr;
    RHIResourceSet*    _resourceSet    = nullptr;
    RHIBuffer*         _perViewBuffer  = nullptr;
    RHIBuffer*         _vertexBuffer   = nullptr;

    std::vector<DebugLineVertex> _vertices;
    std::unordered_map<size_t, RHIGraphicsPipeline*> _pipelineCache;
    uint32 _vertexCapacity = 0;
    uint32 _vertexCount    = 0;
    bool _isInitialized    = false;
};

HS_NS_END

#endif /* __HS_RENDERER_RENDER_PASS_FORWARD_DEBUG_PASS_H__ */
