//
// MultipleScattering.comp.slang
// Compute shader for multiple scattering computation
//
// Computes the nth-order scattered radiance by integrating the scattering density
// along the view ray.
//
#include "AtmosphereCommon.hlsli"

// Atmosphere parameters uniform buffer
ConstantBuffer<AtmosphereParametersGPU> atmosphere;

// Input textures
Texture2D<float4> TransmittanceTexture;
Texture3D<float4> ScatteringDensityTexture;
SamplerState LinearSampler;

// Output scattering textures
RWTexture3D<float4> DeltaMultipleScatteringTexture;  // This order's delta
RWTexture3D<float4> ScatteringTexture;               // Accumulated scattering

// Number of integration samples
static const int SCATTERING_INTEGRAL_SAMPLES = 50;

// Get transmittance between two points
float3 GetTransmittance(float r, float mu, float d, bool rayIntersectsGround)
{
    float r_d = ClampRadius(atmosphere, sqrt(d * d + 2.0 * r * mu * d + r * r));
    float mu_d = ClampCosine((r * mu + d) / r_d);

    float2 uvStart = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
    float2 uvEnd = GetTransmittanceTextureUvFromRMu(atmosphere, r_d, mu_d);

    float3 transStart = TransmittanceTexture.SampleLevel(LinearSampler, uvStart, 0).rgb;
    float3 transEnd = TransmittanceTexture.SampleLevel(LinearSampler, uvEnd, 0).rgb;

    if (rayIntersectsGround)
    {
        return min(transStart / max(transEnd, 0.0001), 1.0);
    }
    else
    {
        return min(transEnd / max(transStart, 0.0001), 1.0);
    }
}

// Get scattering density at a point
float3 GetScatteringDensity(float r, float mu, float mu_s, float nu, bool rayIntersectsGround)
{
    float3 uvw = GetScatteringTextureUvwFromRMuMuSNu(atmosphere, r, mu, mu_s, nu, rayIntersectsGround);
    return ScatteringDensityTexture.SampleLevel(LinearSampler, uvw, 0).rgb;
}

// Compute multiple scattering by integrating scattering density along ray
float3 ComputeMultipleScattering(float r, float mu, float mu_s, float nu, bool rayIntersectsGround)
{
    // Ray length
    float rayLength = rayIntersectsGround
        ? DistanceToBottomAtmosphereBoundary(atmosphere, r, mu)
        : DistanceToTopAtmosphereBoundary(atmosphere, r, mu);

    float dx = rayLength / float(SCATTERING_INTEGRAL_SAMPLES);

    float3 result = float3(0.0, 0.0, 0.0);

    // Trapezoidal integration
    for (int i = 0; i <= SCATTERING_INTEGRAL_SAMPLES; i++)
    {
        float d_i = float(i) * dx;

        // Position along ray
        float r_i = ClampRadius(atmosphere, sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r));
        float mu_i = ClampCosine((r * mu + d_i) / r_i);
        float mu_s_i = ClampCosine((r * mu_s + d_i * nu) / r_i);

        // Transmittance from camera to sample
        float3 transmittance_i = GetTransmittance(r, mu, d_i, rayIntersectsGround);

        // Scattering density at sample
        float3 scatteringDensity_i = GetScatteringDensity(r_i, mu_i, mu_s_i, nu, rayIntersectsGround);

        float weight = (i == 0 || i == SCATTERING_INTEGRAL_SAMPLES) ? 0.5 : 1.0;
        result += transmittance_i * scatteringDensity_i * weight * dx;
    }

    return result;
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

    // Convert to UVW
    float3 uvw = (float3(dispatchThreadID) + 0.5) /
                 float3(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH);

    // Get (r, mu, mu_s, nu)
    float r, mu, mu_s, nu;
    bool rayIntersectsGround;
    GetRMuMuSNuFromScatteringTextureUvw(atmosphere, uvw, r, mu, mu_s, nu, rayIntersectsGround);

    // Clamp nu to valid range
    nu = clamp(nu, mu * mu_s - sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)),
                   mu * mu_s + sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)));

    // Compute multiple scattering
    float3 multipleScattering = ComputeMultipleScattering(r, mu, mu_s, nu, rayIntersectsGround);

    // Write delta (this order's contribution)
    DeltaMultipleScatteringTexture[dispatchThreadID] = float4(multipleScattering, 1.0);

    // Accumulate into total scattering (add to existing)
    float4 existingScattering = ScatteringTexture[dispatchThreadID];
    ScatteringTexture[dispatchThreadID] = float4(existingScattering.rgb + multipleScattering, existingScattering.a);
}
