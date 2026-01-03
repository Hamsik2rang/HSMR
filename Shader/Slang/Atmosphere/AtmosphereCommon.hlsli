//
// AtmosphereCommon.hlsli
// Common definitions and functions for Precomputed Atmospheric Scattering
//
// Based on: Eric Bruneton & Fabrice Neyret, "Precomputed Atmospheric Scattering"
// Reference: github.com/ebruneton/precomputed_atmospheric_scattering
//
#ifndef __ATMOSPHERE_COMMON_HLSLI__
#define __ATMOSPHERE_COMMON_HLSLI__

// =============================================================================
// Constants
// =============================================================================

static const float PI = 3.14159265358979323846;

// Texture dimensions (must match C++ AtmosphereConstants)
static const int TRANSMITTANCE_TEXTURE_WIDTH  = 256;
static const int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

static const int SCATTERING_TEXTURE_R_SIZE    = 32;
static const int SCATTERING_TEXTURE_MU_SIZE   = 128;
static const int SCATTERING_TEXTURE_MU_S_SIZE = 32;
static const int SCATTERING_TEXTURE_NU_SIZE   = 8;

static const int SCATTERING_TEXTURE_WIDTH  = 256;  // NU * MU_S
static const int SCATTERING_TEXTURE_HEIGHT = 128;
static const int SCATTERING_TEXTURE_DEPTH  = 32;

static const int IRRADIANCE_TEXTURE_WIDTH  = 64;
static const int IRRADIANCE_TEXTURE_HEIGHT = 16;

// =============================================================================
// Atmosphere Parameters (GPU version)
// =============================================================================

struct DensityProfileLayerGPU
{
    float width;
    float expTerm;
    float expScale;
    float linearTerm;
    float constantTerm;
    float _padding0;
    float _padding1;
    float _padding2;
};

struct AtmosphereParametersGPU
{
    // Solar
    float3 solarIrradiance;
    float sunAngularRadius;

    // Planet
    float bottomRadius;
    float topRadius;
    float2 _pad0;

    // Rayleigh
    DensityProfileLayerGPU rayleighDensity[2];
    float3 rayleighScattering;
    float _pad1;

    // Mie
    DensityProfileLayerGPU mieDensity[2];
    float3 mieScattering;
    float miePhaseFunctionG;
    float3 mieExtinction;
    float _pad2;

    // Absorption
    DensityProfileLayerGPU absorptionDensity[2];
    float3 absorptionExtinction;
    float _pad3;

    // Ground
    float3 groundAlbedo;
    float muSMin;
};

// =============================================================================
// Utility Functions
// =============================================================================

// Clamp cosine to valid range
float ClampCosine(float mu)
{
    return clamp(mu, -1.0, 1.0);
}

// Clamp distance to positive
float ClampDistance(float d)
{
    return max(d, 0.0);
}

// Clamp radius to atmosphere bounds
float ClampRadius(AtmosphereParametersGPU atmosphere, float r)
{
    return clamp(r, atmosphere.bottomRadius, atmosphere.topRadius);
}

// Safe sqrt that clamps negative values
float SafeSqrt(float x)
{
    return sqrt(max(x, 0.0));
}

// =============================================================================
// Geometry Functions
// =============================================================================

// Calculate distance to top atmosphere boundary
// r: radius from planet center
// mu: cosine of zenith angle
float DistanceToTopAtmosphereBoundary(AtmosphereParametersGPU atmosphere, float r, float mu)
{
    float discriminant = r * r * (mu * mu - 1.0) + atmosphere.topRadius * atmosphere.topRadius;
    return ClampDistance(-r * mu + SafeSqrt(discriminant));
}

// Calculate distance to bottom atmosphere boundary (ground)
float DistanceToBottomAtmosphereBoundary(AtmosphereParametersGPU atmosphere, float r, float mu)
{
    float discriminant = r * r * (mu * mu - 1.0) + atmosphere.bottomRadius * atmosphere.bottomRadius;
    return ClampDistance(-r * mu - SafeSqrt(discriminant));
}

// Check if ray from (r, mu) intersects ground
bool RayIntersectsGround(AtmosphereParametersGPU atmosphere, float r, float mu)
{
    return mu < 0.0 && r * r * (mu * mu - 1.0) + atmosphere.bottomRadius * atmosphere.bottomRadius >= 0.0;
}

// Get layer density at given altitude
float GetLayerDensity(DensityProfileLayerGPU layer, float altitude)
{
    float density = layer.expTerm * exp(layer.expScale * altitude) +
                   layer.linearTerm * altitude + layer.constantTerm;
    return clamp(density, 0.0, 1.0);
}

// Get profile density considering both layers
float GetProfileDensity(DensityProfileLayerGPU layers[2], float altitude)
{
    return altitude < layers[0].width
        ? GetLayerDensity(layers[0], altitude)
        : GetLayerDensity(layers[1], altitude);
}

// =============================================================================
// Phase Functions
// =============================================================================

// Rayleigh phase function
float RayleighPhaseFunction(float nu)
{
    float k = 3.0 / (16.0 * PI);
    return k * (1.0 + nu * nu);
}

// Mie phase function (Henyey-Greenstein approximation)
float MiePhaseFunction(float g, float nu)
{
    float k = 3.0 / (8.0 * PI) * (1.0 - g * g) / (2.0 + g * g);
    return k * (1.0 + nu * nu) / pow(1.0 + g * g - 2.0 * g * nu, 1.5);
}

// =============================================================================
// Transmittance Texture Mapping
// =============================================================================

// Map (r, mu) to transmittance texture UV
float2 GetTransmittanceTextureUvFromRMu(AtmosphereParametersGPU atmosphere, float r, float mu)
{
    float H = sqrt(atmosphere.topRadius * atmosphere.topRadius -
                   atmosphere.bottomRadius * atmosphere.bottomRadius);

    float rho = SafeSqrt(r * r - atmosphere.bottomRadius * atmosphere.bottomRadius);

    float d = DistanceToTopAtmosphereBoundary(atmosphere, r, mu);
    float d_min = atmosphere.topRadius - r;
    float d_max = rho + H;

    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;

    return float2(x_mu, x_r);
}

// Map transmittance texture UV to (r, mu)
void GetRMuFromTransmittanceTextureUv(AtmosphereParametersGPU atmosphere, float2 uv,
                                       out float r, out float mu)
{
    float x_mu = uv.x;
    float x_r = uv.y;

    float H = sqrt(atmosphere.topRadius * atmosphere.topRadius -
                   atmosphere.bottomRadius * atmosphere.bottomRadius);

    float rho = H * x_r;
    r = sqrt(rho * rho + atmosphere.bottomRadius * atmosphere.bottomRadius);

    float d_min = atmosphere.topRadius - r;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);

    mu = (d == 0.0) ? 1.0 : (H * H - rho * rho - d * d) / (2.0 * r * d);
    mu = ClampCosine(mu);
}

// =============================================================================
// Scattering Texture Mapping (4D -> 3D)
// =============================================================================

// Map (r, mu, mu_s, nu) to scattering texture UVW
float3 GetScatteringTextureUvwFromRMuMuSNu(AtmosphereParametersGPU atmosphere,
                                            float r, float mu, float mu_s, float nu,
                                            bool rayRMuIntersectsGround)
{
    float H = sqrt(atmosphere.topRadius * atmosphere.topRadius -
                   atmosphere.bottomRadius * atmosphere.bottomRadius);
    float rho = SafeSqrt(r * r - atmosphere.bottomRadius * atmosphere.bottomRadius);

    float u_r = rho / H;

    float r_mu = r * mu;
    float discriminant = r_mu * r_mu - r * r + atmosphere.bottomRadius * atmosphere.bottomRadius;
    float u_mu;

    if (rayRMuIntersectsGround)
    {
        float d = -r_mu - SafeSqrt(discriminant);
        float d_min = r - atmosphere.bottomRadius;
        float d_max = rho;
        u_mu = 0.5 - 0.5 * (d_max == d_min ? 0.0 : (d - d_min) / (d_max - d_min));
    }
    else
    {
        float d = -r_mu + SafeSqrt(discriminant + H * H);
        float d_min = atmosphere.topRadius - r;
        float d_max = rho + H;
        u_mu = 0.5 + 0.5 * (d_max == d_min ? 0.0 : (d - d_min) / (d_max - d_min));
    }

    float d = DistanceToTopAtmosphereBoundary(atmosphere, atmosphere.bottomRadius, mu_s);
    float d_min = atmosphere.topRadius - atmosphere.bottomRadius;
    float d_max = H;
    float a = (d - d_min) / (d_max - d_min);
    float A = -2.0 * atmosphere.muSMin * atmosphere.bottomRadius / (d_max - d_min);
    float u_mu_s = max(1.0 - a / A, 0.0) / (1.0 + a);

    float u_nu = (nu + 1.0) / 2.0;

    // Pack 4D into 3D
    return float3(
        (u_nu + float(int(u_mu_s * SCATTERING_TEXTURE_MU_S_SIZE))) / SCATTERING_TEXTURE_MU_S_SIZE,
        u_mu,
        u_r
    );
}

// Map scattering texture UVW to (r, mu, mu_s, nu)
void GetRMuMuSNuFromScatteringTextureUvw(AtmosphereParametersGPU atmosphere, float3 uvw,
                                          out float r, out float mu, out float mu_s, out float nu,
                                          out bool rayRMuIntersectsGround)
{
    float H = sqrt(atmosphere.topRadius * atmosphere.topRadius -
                   atmosphere.bottomRadius * atmosphere.bottomRadius);

    float rho = H * uvw.z;
    r = sqrt(rho * rho + atmosphere.bottomRadius * atmosphere.bottomRadius);

    if (uvw.y < 0.5)
    {
        float d_min = r - atmosphere.bottomRadius;
        float d_max = rho;
        float d = d_min + (d_max - d_min) * (1.0 - 2.0 * uvw.y);
        mu = (d == 0.0) ? -1.0 : ClampCosine(-(rho * rho + d * d) / (2.0 * r * d));
        rayRMuIntersectsGround = true;
    }
    else
    {
        float d_min = atmosphere.topRadius - r;
        float d_max = rho + H;
        float d = d_min + (d_max - d_min) * (2.0 * uvw.y - 1.0);
        mu = (d == 0.0) ? 1.0 : ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d));
        rayRMuIntersectsGround = false;
    }

    float x_mu_s = uvw.x * SCATTERING_TEXTURE_MU_S_SIZE;
    float u_mu_s = fmod(x_mu_s, 1.0);
    float u_nu = fmod(x_mu_s, 1.0);

    float d_min = atmosphere.topRadius - atmosphere.bottomRadius;
    float d_max = H;
    float A = -2.0 * atmosphere.muSMin * atmosphere.bottomRadius / (d_max - d_min);
    float a = (A - u_mu_s * A) / (1.0 + u_mu_s * A);
    float d = d_min + min(a, A) * (d_max - d_min);
    mu_s = (d == 0.0) ? 1.0 : ClampCosine((H * H - d * d) / (2.0 * atmosphere.bottomRadius * d));

    nu = ClampCosine(u_nu * 2.0 - 1.0);
}

// =============================================================================
// Irradiance Texture Mapping
// =============================================================================

// Map (r, mu_s) to irradiance texture UV
float2 GetIrradianceTextureUvFromRMuS(AtmosphereParametersGPU atmosphere, float r, float mu_s)
{
    float x_r = (r - atmosphere.bottomRadius) / (atmosphere.topRadius - atmosphere.bottomRadius);
    float x_mu_s = mu_s * 0.5 + 0.5;
    return float2(x_mu_s, x_r);
}

// Map irradiance texture UV to (r, mu_s)
void GetRMuSFromIrradianceTextureUv(AtmosphereParametersGPU atmosphere, float2 uv,
                                     out float r, out float mu_s)
{
    float x_mu_s = uv.x;
    float x_r = uv.y;
    r = atmosphere.bottomRadius + x_r * (atmosphere.topRadius - atmosphere.bottomRadius);
    mu_s = ClampCosine(2.0 * x_mu_s - 1.0);
}

// =============================================================================
// Optical Depth Integration
// =============================================================================

// Number of integration samples for transmittance computation
static const int TRANSMITTANCE_INTEGRAL_SAMPLES = 500;

// Compute optical length along ray for a specific component
float ComputeOpticalLengthToTopAtmosphereBoundary(
    AtmosphereParametersGPU atmosphere,
    DensityProfileLayerGPU profile[2],
    float r, float mu)
{
    float dx = DistanceToTopAtmosphereBoundary(atmosphere, r, mu) / float(TRANSMITTANCE_INTEGRAL_SAMPLES);
    float result = 0.0;

    for (int i = 0; i <= TRANSMITTANCE_INTEGRAL_SAMPLES; i++)
    {
        float d_i = float(i) * dx;
        float r_i = sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r);
        float altitude_i = r_i - atmosphere.bottomRadius;
        float weight_i = (i == 0 || i == TRANSMITTANCE_INTEGRAL_SAMPLES) ? 0.5 : 1.0;
        result += GetProfileDensity(profile, altitude_i) * weight_i * dx;
    }

    return result;
}

// Compute transmittance to top atmosphere boundary
float3 ComputeTransmittanceToTopAtmosphereBoundary(AtmosphereParametersGPU atmosphere, float r, float mu)
{
    float3 rayleighOpticalLength = atmosphere.rayleighScattering *
        ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.rayleighDensity, r, mu);

    float3 mieOpticalLength = atmosphere.mieExtinction *
        ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.mieDensity, r, mu);

    float3 absorptionOpticalLength = atmosphere.absorptionExtinction *
        ComputeOpticalLengthToTopAtmosphereBoundary(atmosphere, atmosphere.absorptionDensity, r, mu);

    return exp(-(rayleighOpticalLength + mieOpticalLength + absorptionOpticalLength));
}

#endif // __ATMOSPHERE_COMMON_HLSLI__
