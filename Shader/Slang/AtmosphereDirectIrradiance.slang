module AtmosphereDirectIrradiance;
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
    if (dispatchThreadID.x >= IRRADIANCE_WIDTH || dispatchThreadID.y >= IRRADIANCE_HEIGHT)
    {
        return;
    }

    float2 uv = (float2(dispatchThreadID.xy) + 0.5f) / float2(IRRADIANCE_WIDTH, IRRADIANCE_HEIGHT);
    float sunMu = uv.x * 2.0f - 1.0f;
    float height = uv.y * atmosphereSettings.planetRadii.z;
    float3 trans = transmittanceFromHeightMu(height, sunMu, atmosphereSettings);
    float sunVisibility = saturate1(sunMu * 0.5f + 0.5f);
    float3 irradiance = trans * atmosphereSettings.sunDirectionIntensity.w * sunVisibility;
    irradianceLut[dispatchThreadID.xy] = float4(irradiance, 1.0f);
}
