//
//  AtmosphereRenderer.h
//  Engine/Renderer/Atmosphere
//
//  Created for HSMR Engine.
//
//  High-level atmosphere rendering system that combines precomputation
//  and real-time sky rendering.
//
#ifndef __HS_ATMOSPHERE_RENDERER_H__
#define __HS_ATMOSPHERE_RENDERER_H__

#include "Precompile.h"
#include "Engine/Renderer/Atmosphere/AtmosphereParameters.h"
#include "Core/ThirdParty/glm/glm.hpp"

HS_NS_BEGIN

class RHIContext;
class RHITexture;
class RHIBuffer;
class RHISampler;
class RHICommandBuffer;
class AtmospherePrecompute;

// =============================================================================
// AtmosphereRenderer
// =============================================================================
// Manages atmospheric rendering including LUT precomputation and sky rendering.
//
// Usage:
//   AtmosphereRenderer renderer;
//   renderer.Initialize(context, EarthAtmosphere::CreateDefault());
//   renderer.Precompute(cmdBuffer);  // One-time or when parameters change
//
//   // In render loop:
//   renderer.GetTransmittanceTexture(), etc. for shader binding

class HS_API AtmosphereRenderer
{
public:
	// Rendering uniforms passed to sky shader
	struct SkyUniforms
	{
		glm::mat4 inverseViewProjection;
		glm::vec3 cameraPosition;    // World space (planet center = origin)
		float     _pad0;
		glm::vec3 sunDirection;      // Normalized direction to sun
		float     sunAngularRadius;
		glm::vec3 earthCenter;       // Planet center in world space
		float     exposure;
		glm::vec3 whitePoint;        // For tone mapping
		float     _pad1;
	};

public:
	AtmosphereRenderer();
	~AtmosphereRenderer();

	// Initialize with atmosphere parameters
	bool Initialize(RHIContext* context, const AtmosphereParameters& params,
	                int numScatteringOrders = AtmosphereConstants::DEFAULT_NUM_SCATTERING_ORDERS);

	// Release all resources
	void Finalize();

	// Precompute LUT textures (call once or when parameters change)
	void Precompute(RHICommandBuffer* cmdBuffer);

	// Check if precomputation is complete
	bool IsPrecomputed() const;

	// Access LUT textures for rendering
	RHITexture* GetTransmittanceTexture() const;
	RHITexture* GetScatteringTexture() const;
	RHITexture* GetIrradianceTexture() const;
	RHITexture* GetSingleMieScatteringTexture() const;

	// Access sampler for LUT textures
	RHISampler* GetLUTSampler() const { return _lutSampler; }

	// Get atmosphere parameters
	const AtmosphereParameters& GetParameters() const;

	// Update sun direction (normalized)
	void SetSunDirection(const glm::vec3& direction) { _sunDirection = glm::normalize(direction); }
	const glm::vec3& GetSunDirection() const { return _sunDirection; }

	// Exposure for tone mapping
	void SetExposure(float exposure) { _exposure = exposure; }
	float GetExposure() const { return _exposure; }

	// White point for tone mapping
	void SetWhitePoint(const glm::vec3& whitePoint) { _whitePoint = whitePoint; }
	const glm::vec3& GetWhitePoint() const { return _whitePoint; }

	// Get the RHI context
	RHIContext* GetRHIContext() const { return _context; }

private:
	RHIContext* _context = nullptr;
	AtmospherePrecompute* _precompute = nullptr;
	RHISampler* _lutSampler = nullptr;

	// Rendering parameters
	glm::vec3 _sunDirection = glm::normalize(glm::vec3(0.0f, 0.707f, 0.707f));
	float _exposure = 10.0f;
	glm::vec3 _whitePoint = glm::vec3(1.08241f, 0.96756f, 0.95003f);
};

HS_NS_END

#endif /* __HS_ATMOSPHERE_RENDERER_H__ */
