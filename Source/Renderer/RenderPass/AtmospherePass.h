//
//  AtmospherePass.h
//  HSMR
//
#ifndef __HS_RENDERER_ATMOSPHERE_PASS_H__
#define __HS_RENDERER_ATMOSPHERE_PASS_H__

#include "Precompile.h"

#include "Renderer/AtmosphereDefinition.h"
#include "Renderer/RendererDefinition.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include <array>
#include <unordered_map>

HS_NS_BEGIN

class RHIBuffer;
class RHICommandBuffer;
class RHIComputePipeline;
class RHIGraphicsPipeline;
class RHIResourceLayout;
class RHIResourceSet;
class RHISampler;
class RHIShader;
class RHITexture;
class RHIContext;
class ShaderLibrary;

struct HS_RENDERER_API AtmosphereLutResources
{
    RHIBuffer* settingsBuffer = nullptr;
    RHITexture* transmittanceLut = nullptr;
    RHITexture* irradianceLut = nullptr;
    RHITexture* scatteringLut = nullptr;
    RHISampler* lutSampler2D = nullptr;
    RHISampler* lutSampler3D = nullptr;

    bool IsValid() const
    {
        return settingsBuffer && transmittanceLut && irradianceLut && scatteringLut && lutSampler2D && lutSampler3D;
    }
};

class HS_RENDERER_API AtmospherePass
{
public:
    AtmospherePass() = default;
    ~AtmospherePass();

    bool Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext);
    void Shutdown();

    bool IsInitialized() const { return _isInitialized; }
    void UpdateSettings(const AtmosphereSettings& settings);
    void PrecomputeIfNeeded(RHICommandBuffer& commandBuffer);

    RHIGraphicsPipeline* GetOrCreateSkyPipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                                RHIBuffer* perViewBuffer);
    RHIResourceSet* GetSkyResourceSet() const { return _skyResourceSet; }
    const AtmosphereLutResources& GetLutResources() const { return _lutResources; }

private:
    struct AtmosphereSettingsUBO
    {
        glm::vec4 planetRadii;
        glm::vec4 densityParams;
        glm::vec4 scatteringParams;
        glm::vec4 sunDirectionIntensity;
        glm::vec4 debugExposureOrder;
    };

    struct ComputeStage
    {
        const char* shaderName = nullptr;
        RHIShader* shader = nullptr;
        RHIComputePipeline* pipeline = nullptr;
        uint32 groupX = 1;
        uint32 groupY = 1;
        uint32 groupZ = 1;
    };

    bool createLutResources();
    bool createComputeResources(ShaderLibrary* shaderLibrary);
    bool createSkyShaders(ShaderLibrary* shaderLibrary);
    void destroyGpuResources();
    void destroyComputeResources();
    void destroySkyResources();
    void invalidateSkyPipelines();
    void rebuildSkyResourceBindings(RHIBuffer* perViewBuffer);
    bool settingsEqual(const AtmosphereSettings& lhs, const AtmosphereSettings& rhs) const;
    AtmosphereSettingsUBO buildSettingsUBO(const AtmosphereSettings& settings) const;

    RHIContext* _rhiContext = nullptr;
    bool _isInitialized = false;
    bool _settingsDirty = true;
    bool _hasSettings = false;
    AtmosphereSettings _settings{};

    AtmosphereLutResources _lutResources{};

    RHIResourceLayout* _computeResourceLayout = nullptr;
    RHIResourceSet* _computeResourceSet = nullptr;
    std::array<ComputeStage, 6> _computeStages{};

    RHIShader* _skyVertexShader = nullptr;
    RHIShader* _skyFragmentShader = nullptr;
    ShaderReflectionDataEx _skyReflection{};
    RHIResourceLayout* _skyResourceLayout = nullptr;
    RHIResourceSet* _skyResourceSet = nullptr;
    RHIBuffer* _perViewBuffer = nullptr;
    std::unordered_map<size_t, RHIGraphicsPipeline*> _skyPipelineCache;
};

HS_NS_END

#endif // __HS_RENDERER_ATMOSPHERE_PASS_H__
