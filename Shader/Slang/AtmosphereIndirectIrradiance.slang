module AtmosphereIndirectIrradiance;
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
    float height01 = uv.y;
    float height = height01 * atmosphereSettings.planetRadii.z;
    float3 direct = transmittanceFromHeightMu(height, sunMu, atmosphereSettings) *
                    atmosphereSettings.sunDirectionIntensity.w *
                    saturate1(sunMu * 0.5f + 0.5f);
    float rayleighDensity = densityRayleigh(height, atmosphereSettings);
    float mieDensity = densityMie(height, atmosphereSettings);
    float3 sky = (rayleighScattering(atmosphereSettings) * rayleighDensity +
                  mieScattering(atmosphereSettings) * mieDensity) *
                 atmosphereSettings.sunDirectionIntensity.w *
                 lerp(12000.0f, 32000.0f, saturate1(sunMu * 0.5f + 0.5f));
    float order = max(atmosphereSettings.debugExposureOrder.z, 1.0f);
    float multipleBoost = 0.08f * (order - 1.0f);
    irradianceLut[dispatchThreadID.xy] = float4(direct + sky * multipleBoost, 1.0f);
}
