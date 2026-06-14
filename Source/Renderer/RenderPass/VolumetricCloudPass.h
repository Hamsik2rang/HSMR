//
//  VolumetricCloudPass.h
//  HSMR
//
#ifndef __HS_RENDERER_RENDER_PASS_VOLUMETRIC_CLOUD_PASS_H__
#define __HS_RENDERER_RENDER_PASS_VOLUMETRIC_CLOUD_PASS_H__

#include "Precompile.h"
#include "RHI/RHIDefinition.h"
#include "Renderer/VolumetricCloudDefinition.h"
#include "ShaderSystem/ShaderSystemDefinition.h"

#include <unordered_map>
#include <vector>

HS_NS_BEGIN

class RHIContext;
class RHIShader;
class RHIBuffer;
class RHITexture;
class RHISampler;
class RHIResourceLayout;
class RHIResourceSet;
class RHIGraphicsPipeline;
class ShaderLibrary;
struct AtmosphereLutResources;

class HS_RENDERER_API VolumetricCloudPass
{
public:
    VolumetricCloudPass() = default;
    ~VolumetricCloudPass();

    bool Initialize(ShaderLibrary* shaderLibrary, RHIContext* rhiContext);
    void Shutdown();

    void UpdateSettings(const VolumetricCloudSettings& settings, float timeSeconds);
    void SetAtmosphereResources(const AtmosphereLutResources& resources);

    RHIGraphicsPipeline* GetOrCreatePipeline(const PipelineRenderTargetLayout& renderTargetLayout,
                                             RHIBuffer* perViewBuffer);

    RHIResourceSet* GetResourceSet() const { return _resourceSet; }
    bool IsInitialized() const { return _isInitialized; }

private:
    struct alignas(16) CloudSettingsUBO
    {
        glm::vec4 sunDirectionIntensity;
        glm::vec4 sunColorAmbient;
        glm::vec4 coverageTypePrecipDensity;
        glm::vec4 erosionWindDebugTime;
        glm::vec4 layerAndAtmosphere;
        glm::vec4 sampleAndLighting;
    };

    bool createNoiseResources();
    void destroyGpuResources();
    void rebuildResourceBindings(RHIBuffer* perViewBuffer);
    void invalidatePipelines();

    static std::vector<uint8> buildBaseNoise(uint32 size);
    static std::vector<uint8> buildDetailNoise(uint32 size);
    static std::vector<uint8> buildCurlWeatherTexture(uint32 size);

    RHIContext*        _rhiContext      = nullptr;
    RHIShader*         _vertexShader    = nullptr;
    RHIShader*         _fragmentShader  = nullptr;
    RHIBuffer*         _settingsBuffer  = nullptr;
    RHITexture*        _baseNoise       = nullptr;
    RHITexture*        _detailNoise     = nullptr;
    RHITexture*        _curlWeather     = nullptr;
    RHISampler*        _volumeSampler   = nullptr;
    RHISampler*        _weatherSampler  = nullptr;
    RHIBuffer*         _atmosphereSettingsBuffer = nullptr;
    RHITexture*        _atmosphereTransmittance = nullptr;
    RHITexture*        _atmosphereIrradiance = nullptr;
    RHITexture*        _atmosphereScattering = nullptr;
    RHISampler*        _atmosphereSampler2D = nullptr;
    RHISampler*        _atmosphereSampler3D = nullptr;
    RHIResourceLayout* _resourceLayout  = nullptr;
    RHIResourceSet*    _resourceSet     = nullptr;
    RHIBuffer*         _perViewBuffer   = nullptr;
    ShaderReflectionDataEx _reflection{};

    std::unordered_map<size_t, RHIGraphicsPipeline*> _pipelineCache;
    bool _isInitialized = false;
};

HS_NS_END

#endif // __HS_RENDERER_RENDER_PASS_VOLUMETRIC_CLOUD_PASS_H__
