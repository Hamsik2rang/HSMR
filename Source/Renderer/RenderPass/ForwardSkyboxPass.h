//
//  ForwardSkyboxPass.h
//  HSMR
//
//  Skybox render pass: renders a 6-sided cubemap (or future sphere/equirect) as
//  a fullscreen background behind all opaque geometry.
//
#ifndef __HS_RENDERER_RENDER_PASS_FORWARD_SKYBOX_PASS_H__
#define __HS_RENDERER_RENDER_PASS_FORWARD_SKYBOX_PASS_H__

#include "Precompile.h"
#include "RHI/RHIDefinition.h"

#include <array>
#include <unordered_map>

namespace hs
{
class RHIContext;
class RHIShader;
class RHIBuffer;
class RHITexture;
class RHISampler;
class RHIResourceLayout;
class RHIResourceSet;
class RHIGraphicsPipeline;
class ShaderLibrary;
class Image;
} // namespace hs

HS_NS_BEGIN

// Determines how the skybox texture is sourced.
// Extend this enum and add a corresponding Configure* method when adding new modes.
enum class HS_RENDERER_API ESkyboxMode : uint8
{
    None,        // No skybox
    SixSided,    // 6 separate 2D images assembled into a cubemap (+X -X +Y -Y +Z -Z)
    Cubemap,     // Pre-built cubemap image (future)
    Spheremap,   // Equirectangular / latitude-longitude single image (future)
};

class HS_RENDERER_API ForwardSkyboxPass
{
public:
    ForwardSkyboxPass() = default;
    ~ForwardSkyboxPass();

    bool Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext);
    void Shutdown();

    // Configure the skybox from 6 separate 2D images.
    // Face order matches Vulkan cubemap layers: +X, -X, +Y, -Y, +Z, -Z.
    // All faces must have the same dimensions and pixel format.
    // Returns false if the images are invalid or incompatible.
    bool ConfigureSixSided(const std::array<Image*, 6>& faces);

    // Returns a pipeline compatible with renderTargetLayout, creating it lazily.
    // perViewBuffer must remain valid for the duration of the draw call.
    RHIGraphicsPipeline* GetOrCreatePipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                             RHIBuffer* perViewBuffer);

    RHIResourceSet* GetResourceSet() const { return _resourceSet; }

    bool IsInitialized() const { return _isInitialized; }
    bool HasSkybox()     const { return _mode != ESkyboxMode::None && _cubemapTexture != nullptr; }

private:
    void destroyCubemapResources();
    void rebuildResourceBindings(RHIBuffer* perViewBuffer);

    RHIContext*        _rhiContext      = nullptr;
    RHIShader*         _vertexShader    = nullptr;
    RHIShader*         _fragmentShader  = nullptr;
    RHITexture*        _cubemapTexture  = nullptr;
    RHISampler*        _cubemapSampler  = nullptr;
    RHIResourceLayout* _resourceLayout  = nullptr;
    RHIResourceSet*    _resourceSet     = nullptr;
    RHIBuffer*         _perViewBuffer   = nullptr; // Change-detection only

    std::unordered_map<size_t, RHIGraphicsPipeline*> _pipelineCache;
    ESkyboxMode _mode        = ESkyboxMode::None;
    bool        _isInitialized = false;
};

HS_NS_END

#endif /* __HS_RENDERER_RENDER_PASS_FORWARD_SKYBOX_PASS_H__ */
