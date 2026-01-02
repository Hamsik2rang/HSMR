//
// Transmittance.comp.slang
// Compute shader for transmittance LUT generation
//
// Computes the transmittance T(r, mu) from any point in the atmosphere
// to the top atmosphere boundary.
//
#include "AtmosphereCommon.hlsli"

// Atmosphere parameters uniform buffer
ConstantBuffer<AtmosphereParametersGPU> atmosphere;

// Output transmittance texture (256 x 64)
RWTexture2D<float4> TransmittanceTexture;

[shader("compute")]
[numthreads(8, 8, 1)]
void ComputeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Check bounds
    if (dispatchThreadID.x >= TRANSMITTANCE_TEXTURE_WIDTH ||
        dispatchThreadID.y >= TRANSMITTANCE_TEXTURE_HEIGHT)
    {
        return;
    }

    // Convert pixel coordinates to UV (center of pixel)
    float2 uv = (float2(dispatchThreadID.xy) + 0.5) /
                float2(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);

    // Get (r, mu) from UV coordinates
    float r, mu;
    GetRMuFromTransmittanceTextureUv(atmosphere, uv, r, mu);

    // Compute transmittance to top atmosphere boundary
    float3 transmittance = ComputeTransmittanceToTopAtmosphereBoundary(atmosphere, r, mu);

    // Write result
    TransmittanceTexture[dispatchThreadID.xy] = float4(transmittance, 1.0);
}
