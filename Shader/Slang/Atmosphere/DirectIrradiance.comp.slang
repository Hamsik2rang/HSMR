//
// DirectIrradiance.comp.slang
// Compute shader for direct irradiance LUT generation
//
// Computes the direct solar irradiance E_sun at any point on the ground,
// attenuated by the atmosphere's transmittance.
//
#include "AtmosphereCommon.hlsli"

// Atmosphere parameters uniform buffer
ConstantBuffer<AtmosphereParametersGPU> atmosphere;

// Input transmittance texture
Texture2D<float4> TransmittanceTexture;
SamplerState TransmittanceSampler;

// Output irradiance textures (64 x 16)
RWTexture2D<float4> DeltaIrradianceTexture;  // Temporary delta E
RWTexture2D<float4> IrradianceTexture;       // Accumulated E

// Lookup transmittance from texture
float3 GetTransmittanceToTopAtmosphereBoundary(float r, float mu)
{
    float2 uv = GetTransmittanceTextureUvFromRMu(atmosphere, r, mu);
    return TransmittanceTexture.SampleLevel(TransmittanceSampler, uv, 0).rgb;
}

// Compute direct irradiance at ground level
float3 ComputeDirectIrradiance(float r, float mu_s)
{
    // Irradiance = solar irradiance * transmittance * max(0, mu_s)
    float alpha_s = atmosphere.sunAngularRadius;

    // Average irradiance over solar disc
    float average_cosine_factor = (mu_s < -alpha_s)
        ? 0.0
        : ((mu_s > alpha_s)
            ? mu_s
            : (mu_s + alpha_s) * (mu_s + alpha_s) / (4.0 * alpha_s));

    return atmosphere.solarIrradiance *
           GetTransmittanceToTopAtmosphereBoundary(r, mu_s) *
           average_cosine_factor;
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

    // Convert pixel coordinates to UV (center of pixel)
    float2 uv = (float2(dispatchThreadID.xy) + 0.5) /
                float2(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);

    // Get (r, mu_s) from UV coordinates
    float r, mu_s;
    GetRMuSFromIrradianceTextureUv(atmosphere, uv, r, mu_s);

    // Compute direct irradiance
    float3 directIrradiance = ComputeDirectIrradiance(r, mu_s);

    // Write to delta texture (for later accumulation)
    DeltaIrradianceTexture[dispatchThreadID.xy] = float4(directIrradiance, 1.0);

    // Initialize irradiance texture (first pass, so just copy)
    IrradianceTexture[dispatchThreadID.xy] = float4(directIrradiance, 1.0);
}
