//
// SingleScattering.comp.slang
// Compute shader for single scattering LUT generation
//
// Computes single Rayleigh and Mie scattering at each point in the atmosphere,
// for all view directions and sun positions.
//
#include "AtmosphereCommon.hlsli"

// Atmosphere parameters uniform buffer
ConstantBuffer<AtmosphereParametersGPU> atmosphere;

// Input transmittance texture
Texture2D<float4> TransmittanceTexture;
SamplerState TransmittanceSampler;

// Output 3D scattering textures (256 x 128 x 32)
RWTexture3D<float4> DeltaRayleighScatteringTexture;
RWTexture3D<float4> DeltaMieScatteringTexture;
RWTexture3D<float4> ScatteringTexture;

// Optional separate Mie storage
RWTexture3D<float4> SingleMieScatteringTexture;

// Configuration - hardcode to not combine for now (no push constants)
static const int combineScatteringTextures = 0;

// Number of integration samples for single scattering
static const int SCATTERING_INTEGRAL_SAMPLES = 50;

// Lookup transmittance from texture
float3 GetTransmittance(float r, float mu, float d, bool rayIntersectsGround)
{
    float r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
    float mu_d = ClampCosine((r * mu + d) / r_d);

    float2 uvStart = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
    float2 uvEnd = GetTransmittanceTextureUvFromRMu(atmosphere, r_d, mu_d);

    float3 transStart = TransmittanceTexture.SampleLevel(TransmittanceSampler, uvStart, 0).rgb;
    float3 transEnd = TransmittanceTexture.SampleLevel(TransmittanceSampler, uvEnd, 0).rgb;

    // Transmittance from p to p_d = T(p) / T(p_d)
    // But we need to handle ground intersection case
    if (rayIntersectsGround)
    {
        return min(transStart / max(transEnd, 0.0001), 1.0);
    }
    else
    {
        return min(transEnd / max(transStart, 0.0001), 1.0);
    }
}

// Get transmittance to sun
float3 GetTransmittanceToSun(float r, float mu_s)
{
    float sin_theta_h = atmosphere.bottomRadius / r;
    float cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0));

    // Smooth horizon transition
    float visibility = smoothstep(-sin_theta_h * atmosphere.sunAngularRadius,
                                   sin_theta_h * atmosphere.sunAngularRadius,
                                   mu_s - cos_theta_h);

    float2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu_s);
    return TransmittanceTexture.SampleLevel(TransmittanceSampler, uv, 0).rgb * visibility;
}

// Compute single scattering integrand
void ComputeSingleScatteringIntegrand(
    float r, float mu, float mu_s, float nu, float d,
    bool rayIntersectsGround,
    out float3 rayleigh, out float3 mie)
{
    float r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
    float mu_s_d = ClampCosine((r * mu_s + d * nu) / r_d);
    float altitude = r_d - atmosphere.bottomRadius;

    // Transmittance from camera to sample point
    float3 transmittance = GetTransmittance(r, mu, d, rayIntersectsGround);

    // Transmittance from sample point to sun
    float3 transmittanceToSun = GetTransmittanceToSun(r_d, mu_s_d);

    // Scattering coefficients at this altitude
    float rayleighDensity = GetProfileDensity(atmosphere.rayleighDensity, altitude);
    float mieDensity = GetProfileDensity(atmosphere.mieDensity, altitude);

    rayleigh = transmittance * transmittanceToSun * rayleighDensity * atmosphere.rayleighScattering;
    mie = transmittance * transmittanceToSun * mieDensity * atmosphere.mieScattering;
}

// Compute single scattering
void ComputeSingleScattering(
    float r, float mu, float mu_s, float nu,
    bool rayIntersectsGround,
    out float3 rayleigh, out float3 mie)
{
    // Integration distance
    float rayLength = rayIntersectsGround
        ? DistanceToBottomAtmosphereBoundary(atmosphere, r, mu)
        : DistanceToTopAtmosphereBoundary(atmosphere, r, mu);

    float dx = rayLength / float(SCATTERING_INTEGRAL_SAMPLES);

    rayleigh = float3(0.0, 0.0, 0.0);
    mie = float3(0.0, 0.0, 0.0);

    // Trapezoidal integration
    for (int i = 0; i <= SCATTERING_INTEGRAL_SAMPLES; i++)
    {
        float d_i = float(i) * dx;

        float3 rayleigh_i, mie_i;
        ComputeSingleScatteringIntegrand(r, mu, mu_s, nu, d_i, rayIntersectsGround,
                                         rayleigh_i, mie_i);

        float weight = (i == 0 || i == SCATTERING_INTEGRAL_SAMPLES) ? 0.5 : 1.0;
        rayleigh += rayleigh_i * weight * dx;
        mie += mie_i * weight * dx;
    }
}

[shader("compute")]
[numthreads(8, 8, 1)]
void ComputeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Check bounds
    if (dispatchThreadID.x >= SCATTERING_TEXTURE_WIDTH ||
        dispatchThreadID.y >= SCATTERING_TEXTURE_HEIGHT ||
        dispatchThreadID.z >= SCATTERING_TEXTURE_DEPTH)
    {
        return;
    }

    // Convert to UVW (center of voxel)
    float3 uvw = (float3(dispatchThreadID) + 0.5) /
                 float3(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH);

    // Get (r, mu, mu_s, nu) from UVW
    float r, mu, mu_s, nu;
    bool rayIntersectsGround;
    GetRMuMuSNuFromScatteringTextureUvw(atmosphere, uvw, r, mu, mu_s, nu, rayIntersectsGround);

    // Clamp nu to valid range based on mu and mu_s
    nu = clamp(nu, mu * mu_s - sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)),
                   mu * mu_s + sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)));

    // Compute single scattering
    float3 rayleigh, mie;
    ComputeSingleScattering(r, mu, mu_s, nu, rayIntersectsGround, rayleigh, mie);

    // Write to delta textures
    DeltaRayleighScatteringTexture[dispatchThreadID] = float4(rayleigh, 1.0);
    DeltaMieScatteringTexture[dispatchThreadID] = float4(mie, 1.0);

    // Apply phase functions and store
    float3 rayleighWithPhase = rayleigh * RayleighPhaseFunction(nu);
    float3 mieWithPhase = mie * MiePhaseFunction(atmosphere.miePhaseFunctionG, nu);

    // Store combined or separate
    if (combineScatteringTextures != 0)
    {
        // Store Rayleigh in RGB, Mie red channel in alpha (for reconstruction)
        ScatteringTexture[dispatchThreadID] = float4(rayleighWithPhase, mieWithPhase.r);
    }
    else
    {
        ScatteringTexture[dispatchThreadID] = float4(rayleighWithPhase, 1.0);
        SingleMieScatteringTexture[dispatchThreadID] = float4(mieWithPhase, 1.0);
    }
}
