module AtmosphereTransmittance;
import AtmosphereCommon;

ConstantBuffer<AtmosphereSettingsUBO> atmosphereSettings;
RWTexture2D<float4> transmittanceLut;
RWTexture2D<float4> irradianceLut;
RWTexture3D<float4> scatteringLut;
Sampler2D transmittanceSampler;
Sampler2D irradianceSampler;
Sampler3D scatteringSampler;

[shader("compute")]
[numthreads(8, 8, 1)]
void ComputeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= TRANSMITTANCE_WIDTH || dispatchThreadID.y >= TRANSMITTANCE_HEIGHT)
    {
        return;
    }

    float2 uv = (float2(dispatchThreadID.xy) + 0.5f) / float2(TRANSMITTANCE_WIDTH, TRANSMITTANCE_HEIGHT);
    float mu = uv.x * 2.0f - 1.0f;
    float height = uv.y * atmosphereSettings.planetRadii.z;
    float3 transmittance = transmittanceFromHeightMu(height, mu, atmosphereSettings);
    transmittanceLut[dispatchThreadID.xy] = float4(transmittance, 1.0f);
}
