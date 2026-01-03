//
// IndirectIrradiance.comp.slang
// Compute shader for indirect irradiance computation (multi-scattering step)
//
// Computes the irradiance received at ground from scattered light
// (i.e., sky irradiance excluding direct sunlight).
//
#include "AtmosphereCommon.hlsli"

// Atmosphere parameters uniform buffer
ConstantBuffer<AtmosphereParametersGPU> atmosphere;

// Input scattering textures
Texture3D<float4> SingleRayleighScatteringTexture;
Texture3D<float4> SingleMieScatteringTexture;
Texture3D<float4> MultipleScatteringTexture;
SamplerState LinearSampler;

// Output irradiance textures
RWTexture2D<float4> DeltaIrradianceTexture;  // This order's contribution
RWTexture2D<float4> IrradianceTexture;       // Accumulated irradiance

// Number of integration samples
static const int IRRADIANCE_INTEGRAL_SAMPLES = 32;

// Get scattering at ground level
// Note: MultipleScatteringTexture should be zeroed for order 2, so no condition needed
float3 GetScattering(float r, float mu, float mu_s, float nu, bool rayIntersectsGround)
{
    float3 uvw = GetScatteringTextureUvwFromRMuMuSNu(atmosphere, r, mu, mu_s, nu, rayIntersectsGround);

    float3 rayleigh = SingleRayleighScatteringTexture.SampleLevel(LinearSampler, uvw, 0).rgb;
    float3 mie = SingleMieScatteringTexture.SampleLevel(LinearSampler, uvw, 0).rgb;
    float3 multipleScattering = MultipleScatteringTexture.SampleLevel(LinearSampler, uvw, 0).rgb;

    // Apply phase functions
    return (rayleigh + multipleScattering) * RayleighPhaseFunction(nu) + mie * MiePhaseFunction(atmosphere.miePhaseFunctionG, nu);
}

// Compute indirect irradiance
float3 ComputeIndirectIrradiance(float r, float mu_s)
{
    float3 result = float3(0.0, 0.0, 0.0);

    float3 omega_s = float3(sqrt(1.0 - mu_s * mu_s), 0.0, mu_s);

    // Integrate over hemisphere
    float dphi = PI / float(IRRADIANCE_INTEGRAL_SAMPLES);
    float dtheta = PI / float(IRRADIANCE_INTEGRAL_SAMPLES);

    for (int j = 0; j < IRRADIANCE_INTEGRAL_SAMPLES / 2; j++)
    {
        float theta = (float(j) + 0.5) * dtheta;

        for (int i = 0; i < 2 * IRRADIANCE_INTEGRAL_SAMPLES; i++)
        {
            float phi = (float(i) + 0.5) * dphi;

            float3 omega = float3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
            float domega = dtheta * dphi * sin(theta);

            float nu = dot(omega, omega_s);

            result += GetScattering(r, omega.z, mu_s, nu, false) * omega.z * domega;
        }
    }

    return result;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void ComputeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Check bounds
    if (dispatchThreadID.x >= IRRADIANCE_TEXTURE_WIDTH ||
        dispatchThreadID.y >= IRRADIANCE_TEXTURE_HEIGHT)
    {
        return;
    }

    // Convert pixel coordinates to UV
    float2 uv = (float2(dispatchThreadID.xy) + 0.5) /
                float2(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);

    // Get (r, mu_s) from UV
    float r, mu_s;
    GetRMuSFromIrradianceTextureUv(atmosphere, uv, r, mu_s);

    // Compute indirect irradiance
    float3 indirectIrradiance = ComputeIndirectIrradiance(r, mu_s);

    // Write delta (this order's contribution)
    DeltaIrradianceTexture[dispatchThreadID.xy] = float4(indirectIrradiance, 1.0);

    // Accumulate into total irradiance
    float3 existingIrradiance = IrradianceTexture[dispatchThreadID.xy].rgb;
    IrradianceTexture[dispatchThreadID.xy] = float4(existingIrradiance + indirectIrradiance, 1.0);
}
