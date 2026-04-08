#ifndef __HS_RENDERER_RENDER_PASS_FORWARD_GRID_PASS_H__
#define __HS_RENDERER_RENDER_PASS_FORWARD_GRID_PASS_H__

#include "Precompile.h"
#include "RHI/RHIDefinition.h"

#include <unordered_map>

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

class HS_RENDERER_API ForwardGridPass
{
public:
    ForwardGridPass() = default;
    ~ForwardGridPass();

    bool Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext);
    void Shutdown();

    // perViewBuffer가 바뀌면 ResourceLayout/Set을 재생성합니다.
    // 첫 호출 시에도 생성됩니다.
    RHIGraphicsPipeline* GetOrCreatePipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                             RHIBuffer* perViewBuffer);

    RHIResourceSet* GetResourceSet() const { return _resourceSet; }
    bool IsInitialized() const { return _isInitialized; }

private:
    void rebuildResourceBindings(RHIBuffer* perViewBuffer);

    RHIContext*        _rhiContext     = nullptr;
    RHIShader*         _vertexShader   = nullptr;
    RHIShader*         _fragmentShader = nullptr;
    RHIResourceLayout* _resourceLayout = nullptr;
    RHIResourceSet*    _resourceSet    = nullptr;
    RHIBuffer*         _perViewBuffer  = nullptr; // 변경 감지용

    std::unordered_map<size_t, RHIGraphicsPipeline*> _pipelineCache;
    bool _isInitialized = false;
};

HS_NS_END

#endif /* __HS_RENDERER_RENDER_PASS_FORWARD_GRID_PASS_H__ */
