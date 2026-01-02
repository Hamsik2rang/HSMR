//
//  AtmosphereParameters.h
//  Engine/Renderer/Atmosphere
//
//  Created by Yongsik Im on 1/2/26.
//
//  Based on: Eric Bruneton & Fabrice Neyret, "Precomputed Atmospheric Scattering"
//  Reference: github.com/ebruneton/precomputed_atmospheric_scattering
//
#ifndef __HS_ATMOSPHERE_PARAMETERS_H__
#define __HS_ATMOSPHERE_PARAMETERS_H__

#include "Precompile.h"
#include "Core/ThirdParty/glm/glm.hpp"

HS_NS_BEGIN

// =============================================================================
// Texture Dimensions (from reference implementation constants.h)
// =============================================================================

namespace AtmosphereConstants
{
	// Transmittance LUT: T(r, mu) - 2D texture
	constexpr int TRANSMITTANCE_TEXTURE_WIDTH  = 256;
	constexpr int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

	// Scattering LUT dimensions (4D packed into 3D)
	// 4D: altitude(r) x view_zenith(mu) x sun_zenith(mu_s) x view_sun_angle(nu)
	constexpr int SCATTERING_TEXTURE_R_SIZE    = 32;   // altitude
	constexpr int SCATTERING_TEXTURE_MU_SIZE   = 128;  // view zenith cosine
	constexpr int SCATTERING_TEXTURE_MU_S_SIZE = 32;   // sun zenith cosine
	constexpr int SCATTERING_TEXTURE_NU_SIZE   = 8;    // view-sun angle

	// 4D -> 3D packing: width = NU * MU_S, height = MU, depth = R
	constexpr int SCATTERING_TEXTURE_WIDTH  = SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE; // 256
	constexpr int SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;  // 128
	constexpr int SCATTERING_TEXTURE_DEPTH  = SCATTERING_TEXTURE_R_SIZE;   // 32

	// Irradiance LUT: E(r, mu_s) - 2D texture
	constexpr int IRRADIANCE_TEXTURE_WIDTH  = 64;
	constexpr int IRRADIANCE_TEXTURE_HEIGHT = 16;

	// Compute shader thread group size
	constexpr int COMPUTE_THREAD_GROUP_SIZE = 8;

	// Default number of scattering orders for multi-scattering
	constexpr int DEFAULT_NUM_SCATTERING_ORDERS = 4;
}

// =============================================================================
// Density Profile Layer
// =============================================================================
// Describes a layer in the atmosphere with specific density characteristics.
// Density at altitude h is computed as:
//   density = exp_term * exp(exp_scale * h) + linear_term * h + constant_term
//
// This allows modeling both exponential (Rayleigh, Mie) and peaked (Ozone) profiles.

struct HS_API DensityProfileLayer
{
	float width;         // Layer thickness (m). Ignored for the last layer.
	float expTerm;       // Exponential coefficient (dimensionless)
	float expScale;      // Exponential scale (m^-1)
	float linearTerm;    // Linear coefficient (m^-1)
	float constantTerm;  // Constant term (dimensionless)

	DensityProfileLayer()
		: width(0.0f)
		, expTerm(0.0f)
		, expScale(0.0f)
		, linearTerm(0.0f)
		, constantTerm(0.0f)
	{
	}

	DensityProfileLayer(float w, float et, float es, float lt, float ct)
		: width(w)
		, expTerm(et)
		, expScale(es)
		, linearTerm(lt)
		, constantTerm(ct)
	{
	}
};

// =============================================================================
// Density Profile
// =============================================================================
// Supports up to 2 layers for complex density profiles (e.g., Ozone layer)

struct HS_API DensityProfile
{
	static constexpr int MAX_LAYERS = 2;
	DensityProfileLayer layers[MAX_LAYERS];

	DensityProfile()
	{
		// Default: single exponential decay layer
		layers[0] = DensityProfileLayer();
		layers[1] = DensityProfileLayer();
	}
};

// =============================================================================
// Atmosphere Parameters
// =============================================================================
// Complete specification of atmospheric properties for rendering.
// All units in SI (meters, radians, W/m^2/nm, etc.)

struct HS_API AtmosphereParameters
{
	// -------------------------------------------------------------------------
	// Solar Parameters
	// -------------------------------------------------------------------------
	glm::vec3 solarIrradiance;    // Top-of-atmosphere solar irradiance (W/m^2/nm)
	float sunAngularRadius;       // Sun angular radius (radians), typically ~0.00935

	// -------------------------------------------------------------------------
	// Planet Parameters
	// -------------------------------------------------------------------------
	float bottomRadius;           // Planet surface radius (m), Earth: 6,360,000
	float topRadius;              // Atmosphere top radius (m), Earth: 6,420,000

	// -------------------------------------------------------------------------
	// Rayleigh Scattering (air molecules)
	// -------------------------------------------------------------------------
	DensityProfile rayleighDensity;
	glm::vec3 rayleighScattering; // Scattering coefficient at sea level (m^-1)

	// -------------------------------------------------------------------------
	// Mie Scattering (aerosols)
	// -------------------------------------------------------------------------
	DensityProfile mieDensity;
	glm::vec3 mieScattering;      // Scattering coefficient at sea level (m^-1)
	glm::vec3 mieExtinction;      // Extinction coefficient at sea level (m^-1)
	float miePhaseFunctionG;      // Asymmetry parameter for Henyey-Greenstein (0.76-0.9)

	// -------------------------------------------------------------------------
	// Absorption (Ozone layer)
	// -------------------------------------------------------------------------
	DensityProfile absorptionDensity;
	glm::vec3 absorptionExtinction; // Extinction coefficient (m^-1)

	// -------------------------------------------------------------------------
	// Ground
	// -------------------------------------------------------------------------
	glm::vec3 groundAlbedo;       // Average ground reflectance
	float muSMin;                 // cos(maximum sun zenith angle)

	AtmosphereParameters()
		: solarIrradiance(1.0f)
		, sunAngularRadius(0.00935f) // ~0.536 degrees
		, bottomRadius(6360000.0f)   // 6360 km
		, topRadius(6420000.0f)      // 6420 km
		, rayleighScattering(5.802e-6f, 13.558e-6f, 33.1e-6f)
		, mieScattering(3.996e-6f)
		, mieExtinction(4.440e-6f)
		, miePhaseFunctionG(0.8f)
		, absorptionExtinction(0.650e-6f, 1.881e-6f, 0.085e-6f)
		, groundAlbedo(0.1f)
		, muSMin(-0.2f) // cos(101.6 degrees) - allow sun slightly below horizon
	{
	}
};

// =============================================================================
// Earth Atmosphere Default Values
// =============================================================================
// Pre-configured parameters for Earth's atmosphere based on measured data.

namespace EarthAtmosphere
{
	// Planet dimensions
	constexpr float BOTTOM_RADIUS = 6360000.0f;  // m (6360 km)
	constexpr float TOP_RADIUS    = 6420000.0f;  // m (6420 km)

	// Rayleigh scattering (air molecules)
	// Scale height: 8 km
	constexpr float RAYLEIGH_SCALE_HEIGHT = 8000.0f;  // m
	// Scattering coefficients at sea level (m^-1)
	// Values correspond to wavelengths ~680nm (R), ~550nm (G), ~440nm (B)
	inline glm::vec3 RayleighScattering()
	{
		return glm::vec3(5.802e-6f, 13.558e-6f, 33.1e-6f);
	}

	// Mie scattering (aerosols)
	// Scale height: 1.2 km
	constexpr float MIE_SCALE_HEIGHT = 1200.0f;  // m
	constexpr float MIE_PHASE_G      = 0.8f;     // Asymmetry parameter
	inline glm::vec3 MieScattering()
	{
		return glm::vec3(3.996e-6f);
	}
	inline glm::vec3 MieExtinction()
	{
		return glm::vec3(4.440e-6f);
	}

	// Ozone absorption
	// Centered at 25 km altitude, +/- 15 km
	constexpr float OZONE_CENTER_ALTITUDE = 25000.0f;  // m
	constexpr float OZONE_WIDTH           = 15000.0f;  // m
	inline glm::vec3 OzoneExtinction()
	{
		return glm::vec3(0.650e-6f, 1.881e-6f, 0.085e-6f);
	}

	// Solar parameters
	constexpr float SUN_ANGULAR_RADIUS = 0.00935f;  // radians (~0.536 degrees)

	// Creates fully configured Earth atmosphere parameters
	inline AtmosphereParameters CreateDefault()
	{
		AtmosphereParameters params;

		// Solar
		params.solarIrradiance  = glm::vec3(1.474f, 1.850f, 1.911f); // Relative to 1 at 550nm
		params.sunAngularRadius = SUN_ANGULAR_RADIUS;

		// Planet
		params.bottomRadius = BOTTOM_RADIUS;
		params.topRadius    = TOP_RADIUS;

		// Rayleigh - exponential decay from sea level
		params.rayleighDensity.layers[0] = DensityProfileLayer(
			0.0f,                          // width (unused for last layer)
			1.0f,                          // expTerm
			-1.0f / RAYLEIGH_SCALE_HEIGHT, // expScale
			0.0f,                          // linearTerm
			0.0f                           // constantTerm
		);
		params.rayleighScattering = RayleighScattering();

		// Mie - exponential decay from sea level
		params.mieDensity.layers[0] = DensityProfileLayer(
			0.0f,                      // width
			1.0f,                      // expTerm
			-1.0f / MIE_SCALE_HEIGHT,  // expScale
			0.0f,                      // linearTerm
			0.0f                       // constantTerm
		);
		params.mieScattering      = MieScattering();
		params.mieExtinction      = MieExtinction();
		params.miePhaseFunctionG  = MIE_PHASE_G;

		// Ozone - tent function centered at 25 km
		// Layer 0: linear increase from 10km to 25km
		params.absorptionDensity.layers[0] = DensityProfileLayer(
			OZONE_WIDTH,              // width: 15 km
			0.0f,                     // expTerm: no exponential component
			0.0f,                     // expScale
			1.0f / OZONE_WIDTH,       // linearTerm: increase to peak
			-2.0f / 3.0f              // constantTerm: offset so density=0 at 10km
		);
		// Layer 1: linear decrease from 25km to 40km
		params.absorptionDensity.layers[1] = DensityProfileLayer(
			0.0f,                     // width (last layer)
			0.0f,                     // expTerm
			0.0f,                     // expScale
			-1.0f / OZONE_WIDTH,      // linearTerm: decrease from peak
			8.0f / 3.0f               // constantTerm: offset so density=0 at 40km
		);
		params.absorptionExtinction = OzoneExtinction();

		// Ground
		params.groundAlbedo = glm::vec3(0.1f);
		params.muSMin       = -0.2f; // Allow twilight rendering

		return params;
	}
}

HS_NS_END

#endif /* __HS_ATMOSPHERE_PARAMETERS_H__ */
