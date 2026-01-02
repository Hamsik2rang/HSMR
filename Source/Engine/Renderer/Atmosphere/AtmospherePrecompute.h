//
//  AtmospherePrecompute.h
//  Engine/Renderer/Atmosphere
//
//  Created by Yongsik Im on 1/2/26.
//
//  Precomputed Atmospheric Scattering lookup table generator.
//  Based on: Eric Bruneton & Fabrice Neyret, "Precomputed Atmospheric Scattering"
//
#ifndef __HS_ATMOSPHERE_PRECOMPUTE_H__
#define __HS_ATMOSPHERE_PRECOMPUTE_H__

#include "Precompile.h"
#include "Engine/Renderer/Atmosphere/AtmosphereParameters.h"

HS_NS_BEGIN

class RHIContext;
class RHITexture;
class RHIBuffer;
class RHIShader;
class RHIComputePipeline;
class RHIResourceLayout;
class RHIResourceSet;
class RHICommandBuffer;
class RHISampler;

// =============================================================================
// AtmospherePrecompute
// =============================================================================
// Generates precomputed atmospheric scattering lookup tables using compute shaders.
//
// Usage:
//   AtmospherePrecompute precompute;
//   precompute.Initialize(context, EarthAtmosphere::CreateDefault());
//   precompute.Compute(commandBuffer);  // Dispatch compute shaders
//   // ... use GetTransmittanceTexture(), GetScatteringTexture(), etc.
//   precompute.Finalize();

class HS_API AtmospherePrecompute
{
public:
	AtmospherePrecompute();
	~AtmospherePrecompute();

	// Initialize resources (textures, shaders, pipelines)
	bool Initialize(RHIContext* context, const AtmosphereParameters& params,
	                int numScatteringOrders = AtmosphereConstants::DEFAULT_NUM_SCATTERING_ORDERS);

	// Finalize and release resources
	void Finalize();

	// Execute precomputation (dispatches compute shaders)
	void Compute(RHICommandBuffer* cmdBuffer);

	// Check if precomputation is complete
	bool IsComputed() const { return _isComputed; }

	// Access final LUT textures (valid after Compute())
	RHITexture* GetTransmittanceTexture() const { return _transmittanceTexture; }
	RHITexture* GetScatteringTexture() const { return _scatteringTexture; }
	RHITexture* GetIrradianceTexture() const { return _irradianceTexture; }
	RHITexture* GetSingleMieScatteringTexture() const { return _singleMieScatteringTexture; }

	// Get atmosphere parameters
	const AtmosphereParameters& GetParameters() const { return _params; }

private:
	// GPU-compatible parameter structure
	struct AtmosphereParametersGPU
	{
		// Solar
		glm::vec3 solarIrradiance;
		float sunAngularRadius;

		// Planet
		float bottomRadius;
		float topRadius;
		float _pad0[2];

		// Rayleigh density profile (2 layers)
		struct DensityLayerGPU
		{
			float width;
			float expTerm;
			float expScale;
			float linearTerm;
			float constantTerm;
			float _pad[3];
		} rayleighDensity[2];
		glm::vec3 rayleighScattering;
		float _pad1;

		// Mie density profile (2 layers)
		DensityLayerGPU mieDensity[2];
		glm::vec3 mieScattering;
		float miePhaseFunctionG;
		glm::vec3 mieExtinction;
		float _pad2;

		// Absorption density profile (2 layers)
		DensityLayerGPU absorptionDensity[2];
		glm::vec3 absorptionExtinction;
		float _pad3;

		// Ground
		glm::vec3 groundAlbedo;
		float muSMin;
	};

	// Convert CPU parameters to GPU format
	void ConvertToGPUFormat(const AtmosphereParameters& params, AtmosphereParametersGPU& gpuParams);

	// Create textures
	bool CreateTextures();
	void DestroyTextures();

	// Create shaders and pipelines
	bool CreateShaders();
	bool CreatePipelines();
	void DestroyShaders();
	void DestroyPipelines();

	// Create resource layouts and sets
	bool CreateResourceLayouts();
	bool CreateResourceSets();
	void DestroyResourceLayouts();
	void DestroyResourceSets();

	// Dispatch individual compute passes
	void DispatchTransmittance(RHICommandBuffer* cmdBuffer);
	void DispatchDirectIrradiance(RHICommandBuffer* cmdBuffer);
	void DispatchSingleScattering(RHICommandBuffer* cmdBuffer);
	void DispatchScatteringDensity(RHICommandBuffer* cmdBuffer, int scatteringOrder);
	void DispatchIndirectIrradiance(RHICommandBuffer* cmdBuffer, int scatteringOrder);
	void DispatchMultipleScattering(RHICommandBuffer* cmdBuffer);

private:
	RHIContext* _context = nullptr;
	AtmosphereParameters _params;
	int _numScatteringOrders = 4;
	bool _isInitialized = false;
	bool _isComputed = false;

	// Parameter buffer
	RHIBuffer* _paramsBuffer = nullptr;

	// Final output textures
	RHITexture* _transmittanceTexture = nullptr;      // 2D: 256 x 64
	RHITexture* _scatteringTexture = nullptr;         // 3D: 256 x 128 x 32
	RHITexture* _irradianceTexture = nullptr;         // 2D: 64 x 16
	RHITexture* _singleMieScatteringTexture = nullptr;// 3D: 256 x 128 x 32 (optional)

	// Temporary textures for multi-scattering computation
	RHITexture* _deltaIrradianceTexture = nullptr;
	RHITexture* _deltaRayleighScatteringTexture = nullptr;
	RHITexture* _deltaMieScatteringTexture = nullptr;
	RHITexture* _deltaScatteringDensityTexture = nullptr;
	RHITexture* _deltaMultipleScatteringTexture = nullptr;

	// Sampler for texture reads
	RHISampler* _linearSampler = nullptr;

	// Compute shaders
	RHIShader* _transmittanceShader = nullptr;
	RHIShader* _directIrradianceShader = nullptr;
	RHIShader* _singleScatteringShader = nullptr;
	RHIShader* _scatteringDensityShader = nullptr;
	RHIShader* _indirectIrradianceShader = nullptr;
	RHIShader* _multipleScatteringShader = nullptr;

	// Compute pipelines
	RHIComputePipeline* _transmittancePipeline = nullptr;
	RHIComputePipeline* _directIrradiancePipeline = nullptr;
	RHIComputePipeline* _singleScatteringPipeline = nullptr;
	RHIComputePipeline* _scatteringDensityPipeline = nullptr;
	RHIComputePipeline* _indirectIrradiancePipeline = nullptr;
	RHIComputePipeline* _multipleScatteringPipeline = nullptr;

	// Resource layouts
	RHIResourceLayout* _transmittanceLayout = nullptr;
	RHIResourceLayout* _directIrradianceLayout = nullptr;
	RHIResourceLayout* _singleScatteringLayout = nullptr;
	RHIResourceLayout* _scatteringDensityLayout = nullptr;
	RHIResourceLayout* _indirectIrradianceLayout = nullptr;
	RHIResourceLayout* _multipleScatteringLayout = nullptr;

	// Resource sets
	RHIResourceSet* _transmittanceSet = nullptr;
	RHIResourceSet* _directIrradianceSet = nullptr;
	RHIResourceSet* _singleScatteringSet = nullptr;
	RHIResourceSet* _scatteringDensitySet = nullptr;
	RHIResourceSet* _indirectIrradianceSet = nullptr;
	RHIResourceSet* _multipleScatteringSet = nullptr;
};

HS_NS_END

#endif /* __HS_ATMOSPHERE_PRECOMPUTE_H__ */
