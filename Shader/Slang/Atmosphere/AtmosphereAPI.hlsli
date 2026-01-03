//
// AtmosphereAPI.hlsli
// Runtime rendering API for Precomputed Atmospheric Scattering
//
// Based on: Eric Bruneton & Fabrice Neyret, "Precomputed Atmospheric Scattering"
// Reference: github.com/ebruneton/precomputed_atmospheric_scattering
//
// This file provides high-level functions for sky rendering using precomputed LUT textures.
//
#ifndef __ATMOSPHERE_API_HLSLI__
#define __ATMOSPHERE_API_HLSLI__

#include "AtmosphereCommon.hlsli"

// =============================================================================
// LUT Texture Declarations (to be bound externally)
// =============================================================================
// These must be declared in the shader that includes this file:
//   Texture2D<float4> TransmittanceTexture;
//   Texture3D<float4> ScatteringTexture;
//   Texture2D<float4> IrradianceTexture;
//   Texture3D<float4> SingleMieScatteringTexture; // Optional
//   SamplerState LUTSampler;

// =============================================================================
// Transmittance Lookup
// =============================================================================

// Get transmittance from LUT
float3 GetTransmittanceToTopAtmosphereBoundary(
    AtmosphereParametersGPU atmosphere,
    Texture2D<float4> transmittanceTexture,
    SamplerState lutSampler,
    float r, float mu)
{
    float2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
    return transmittanceTexture.SampleLevel(lutSampler, uv, 0).rgb;
}

// Get transmittance between two points
float3 GetTransmittance(
    AtmosphereParametersGPU atmosphere,
    Texture2D<float4> transmittanceTexture,
    SamplerState lutSampler,
    float r, float mu, float d, bool rayRMuIntersectsGround)
{
    float r_d = ClampRadius(atmosphere,
        sqrt(d * d + 2.0 * r * mu * d + r * r));
    float mu_d = ClampCosine((r * mu + d) / r_d);

    if (rayRMuIntersectsGround)
    {
        return min(
            GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittanceTexture, lutSampler, r_d, -mu_d) /
            GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittanceTexture, lutSampler, r, -mu),
            float3(1.0, 1.0, 1.0));
    }
    else
    {
        return min(
            GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittanceTexture, lutSampler, r, mu) /
            GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittanceTexture, lutSampler, r_d, mu_d),
            float3(1.0, 1.0, 1.0));
    }
}

// Get transmittance to sun
float3 GetTransmittanceToSun(
    AtmosphereParametersGPU atmosphere,
    Texture2D<float4> transmittanceTexture,
    SamplerState lutSampler,
    float r, float mu_s)
{
    float sin_theta_h = atmosphere.bottomRadius / r;
    float cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0));

    return GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittanceTexture, lutSampler, r, mu_s) *
           smoothstep(-sin_theta_h * atmosphere.sunAngularRadius,
                      sin_theta_h * atmosphere.sunAngularRadius,
                      mu_s - cos_theta_h);
}

// =============================================================================
// Scattering Lookup
// =============================================================================

// Get scattering from 3D LUT (combined Rayleigh + Mie)
float4 GetScatteringTextureValue(
    AtmosphereParametersGPU atmosphere,
    Texture3D<float4> scatteringTexture,
    SamplerState lutSampler,
    float r, float mu, float mu_s, float nu,
    bool rayRMuIntersectsGround)
{
    float3 uvw = GetScatteringTextureUvwFromRMuMuSNu(
        atmosphere, r, mu, mu_s, nu, rayRMuIntersectsGround);
    return scatteringTexture.SampleLevel(lutSampler, uvw, 0);
}

// Get combined scattering (interpolates between two adjacent nu values for higher precision)
float3 GetScattering(
    AtmosphereParametersGPU atmosphere,
    Texture3D<float4> scatteringTexture,
    Texture3D<float4> singleMieScatteringTexture,
    SamplerState lutSampler,
    float r, float mu, float mu_s, float nu,
    bool rayRMuIntersectsGround,
    out float3 singleMieScattering)
{
    float4 uvwz = float4(GetScatteringTextureUvwFromRMuMuSNu(
        atmosphere, r, mu, mu_s, nu, rayRMuIntersectsGround), 0);

    float texCoordX = uvwz.x * float(SCATTERING_TEXTURE_NU_SIZE - 1);
    float texX = floor(texCoordX);
    float lerp_factor = texCoordX - texX;

    float3 uvw0 = float3((texX + uvwz.x * SCATTERING_TEXTURE_MU_S_SIZE) / SCATTERING_TEXTURE_WIDTH,
                         uvwz.y, uvwz.z);
    float3 uvw1 = float3((texX + 1.0 + uvwz.x * SCATTERING_TEXTURE_MU_S_SIZE) / SCATTERING_TEXTURE_WIDTH,
                         uvwz.y, uvwz.z);

    float4 combined0 = scatteringTexture.SampleLevel(lutSampler, uvw0, 0);
    float4 combined1 = scatteringTexture.SampleLevel(lutSampler, uvw1, 0);
    float4 combined = lerp(combined0, combined1, lerp_factor);

    // Extract single Mie from alpha channel (stored as red component of Mie)
    float3 mie0 = singleMieScatteringTexture.SampleLevel(lutSampler, uvw0, 0).rgb;
    float3 mie1 = singleMieScatteringTexture.SampleLevel(lutSampler, uvw1, 0).rgb;
    singleMieScattering = lerp(mie0, mie1, lerp_factor);

    return combined.rgb;
}

// Simplified version that uses scattering texture's alpha for Mie
float3 GetScatteringSimple(
    AtmosphereParametersGPU atmosphere,
    Texture3D<float4> scatteringTexture,
    SamplerState lutSampler,
    float r, float mu, float mu_s, float nu,
    bool rayRMuIntersectsGround,
    out float3 singleMieScattering)
{
    float4 scattering = GetScatteringTextureValue(
        atmosphere, scatteringTexture, lutSampler,
        r, mu, mu_s, nu, rayRMuIntersectsGround);

    // The alpha channel stores the first component of single Mie scattering
    // Reconstruct full Mie using Mie scattering coefficient ratios
    singleMieScattering = scattering.rgb * (scattering.a / scattering.r) *
        (atmosphere.mieScattering / atmosphere.mieScattering.r);

    return scattering.rgb;
}

// =============================================================================
// Irradiance Lookup
// =============================================================================

// Get irradiance from LUT
float3 GetIrradiance(
    AtmosphereParametersGPU atmosphere,
    Texture2D<float4> irradianceTexture,
    SamplerState lutSampler,
    float r, float mu_s)
{
    float2 uv = GetIrradianceTextureUvFromRMuS(atmosphere, r, mu_s);
    return irradianceTexture.SampleLevel(lutSampler, uv, 0).rgb;
}

// =============================================================================
// Sky Radiance API
// =============================================================================

// Main sky rendering function: Get radiance along a view ray
// camera: camera position relative to planet center
// view_ray: normalized view direction
// shadow_length: length of shadow cast by occluding objects (0 for no shadow)
// sun_direction: normalized direction to sun
// transmittance: output transmittance along the ray
float3 GetSkyRadiance(
    AtmosphereParametersGPU atmosphere,
    Texture2D<float4> transmittanceTexture,
    Texture3D<float4> scatteringTexture,
    Texture3D<float4> singleMieScatteringTexture,
    SamplerState lutSampler,
    float3 camera, float3 view_ray, float shadow_length,
    float3 sun_direction, out float3 transmittance)
{
    // Compute the distance to the top atmosphere boundary
    float r = length(camera);
    float rmu = dot(camera, view_ray);
    float distanceToTop = -rmu + sqrt(rmu * rmu - r * r + atmosphere.topRadius * atmosphere.topRadius);

    // If the ray doesn't intersect the atmosphere, return black
    if (distanceToTop < 0.0)
    {
        transmittance = float3(1.0, 1.0, 1.0);
        return float3(0.0, 0.0, 0.0);
    }

    // Move to the atmosphere entry point if we're outside
    if (r > atmosphere.topRadius)
    {
        camera = camera + view_ray * distanceToTop;
        r = atmosphere.topRadius;
        rmu += distanceToTop;
    }

    // Compute mu, mu_s, nu for the current position
    float mu = rmu / r;
    float mu_s = dot(camera, sun_direction) / r;
    float nu = dot(view_ray, sun_direction);
    bool rayRMuIntersectsGround = RayIntersectsGround(atmosphere, r, mu);

    // Get transmittance to the end of the ray
    transmittance = rayRMuIntersectsGround ?
        float3(0.0, 0.0, 0.0) :
        GetTransmittanceToTopAtmosphereBoundary(atmosphere, transmittanceTexture, lutSampler, r, mu);

    // Get single and multiple scattering
    float3 singleMieScattering;
    float3 scattering = GetScattering(
        atmosphere, scatteringTexture, singleMieScatteringTexture, lutSampler,
        r, mu, mu_s, nu, rayRMuIntersectsGround,
        singleMieScattering);

    // Phase functions already applied during precomputation, just add
    float3 radiance = scattering + singleMieScattering;

    return radiance;
}

// Get radiance from camera to a specific point in the atmosphere
// For aerial perspective
float3 GetSkyRadianceToPoint(
    AtmosphereParametersGPU atmosphere,
    Texture2D<float4> transmittanceTexture,
    Texture3D<float4> scatteringTexture,
    Texture3D<float4> singleMieScatteringTexture,
    SamplerState lutSampler,
    float3 camera, float3 point, float shadow_length,
    float3 sun_direction, out float3 transmittance)
{
    float3 view_ray = normalize(point - camera);
    float r = length(camera);
    float rmu = dot(camera, view_ray);
    float d = length(point - camera);

    float mu = rmu / r;
    float mu_s = dot(camera, sun_direction) / r;
    float nu = dot(view_ray, sun_direction);

    bool rayRMuIntersectsGround = RayIntersectsGround(atmosphere, r, mu);

    // Get transmittance from camera to point
    transmittance = GetTransmittance(atmosphere, transmittanceTexture, lutSampler,
        r, mu, d, rayRMuIntersectsGround);

    // Get scattering at camera
    float3 singleMieScattering;
    float3 scattering = GetScatteringSimple(
        atmosphere, scatteringTexture, lutSampler,
        r, mu, mu_s, nu, rayRMuIntersectsGround,
        singleMieScattering);

    // Subtract scattering at point (already accumulated beyond it)
    float r_p = length(point);
    float mu_p = (rmu + d) / r_p;
    float mu_s_p = dot(point, sun_direction) / r_p;

    float3 singleMieScatteringP;
    float3 scatteringP = GetScatteringSimple(
        atmosphere, scatteringTexture, lutSampler,
        r_p, mu_p, mu_s_p, nu, rayRMuIntersectsGround,
        singleMieScatteringP);

    // Compute inscatter between camera and point
    float3 inscatter = (scattering - transmittance * scatteringP) * RayleighPhaseFunction(nu) +
                       (singleMieScattering - transmittance * singleMieScatteringP) *
                       MiePhaseFunction(atmosphere.miePhaseFunctionG, nu);

    return max(inscatter, float3(0.0, 0.0, 0.0));
}

// Get sun and sky irradiance at a surface point
float3 GetSunAndSkyIrradiance(
    AtmosphereParametersGPU atmosphere,
    Texture2D<float4> transmittanceTexture,
    Texture2D<float4> irradianceTexture,
    SamplerState lutSampler,
    float3 point, float3 normal, float3 sun_direction,
    out float3 sky_irradiance)
{
    float r = length(point);
    float mu_s = dot(point, sun_direction) / r;

    // Sky irradiance
    sky_irradiance = GetIrradiance(atmosphere, irradianceTexture, lutSampler, r, mu_s) *
        (1.0 + dot(normal, point) / r) * 0.5;

    // Direct sun irradiance
    float3 sunIrradiance = atmosphere.solarIrradiance *
        GetTransmittanceToSun(atmosphere, transmittanceTexture, lutSampler, r, mu_s) *
        max(dot(normal, sun_direction), 0.0);

    return sunIrradiance;
}

// Get solar radiance (for rendering sun disk)
float3 GetSolarRadiance(AtmosphereParametersGPU atmosphere)
{
    return atmosphere.solarIrradiance / (PI * atmosphere.sunAngularRadius * atmosphere.sunAngularRadius);
}

// =============================================================================
// Tone Mapping
// =============================================================================

// Simple exposure tone mapping with gamma correction
float3 ToneMap(float3 radiance, float exposure, float3 whitePoint)
{
    return pow(float3(1.0, 1.0, 1.0) - exp(-radiance / whitePoint * exposure), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
}

#endif // __ATMOSPHERE_API_HLSLI__
