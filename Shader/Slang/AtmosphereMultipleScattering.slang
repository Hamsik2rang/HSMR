module AtmosphereMultipleScattering;
import AtmosphereCommon;

ConstantBuffer<AtmosphereSettingsUBO> atmosphereSettings;
RWTexture2D<float4> transmittanceLut;
RWTexture2D<float4> irradianceLut;
RWTexture3D<float4> scatteringLut;

[shader("compute")]
[numthreads(8, 8, 4)]
void ComputeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x >= SCATTERING_WIDTH ||
        dispatchThreadID.y >= SCATTERING_HEIGHT ||
        dispatchThreadID.z >= SCATTERING_DEPTH)
    {
        return;
    }

    float3 uvw = (float3(dispatchThreadID) + 0.5f) / float3(SCATTERING_WIDTH, SCATTERING_HEIGHT, SCATTERING_DEPTH);
    float viewMu = uvw.x * 2.0f - 1.0f;
    float sunMu = uvw.y * 2.0f - 1.0f;
    float height = uvw.z * atmosphereSettings.planetRadii.z;
    float nu = saturate1(viewMu * sunMu * 0.5f + 0.5f);

    float3 transView = transmittanceFromHeightMu(height, viewMu, atmosphereSettings);
    float3 irradiance = transmittanceFromHeightMu(height, sunMu, atmosphereSettings) *
                        atmosphereSettings.sunDirectionIntensity.w *
                        saturate1(sunMu * 0.5f + 0.5f);
    float air = densityRayleigh(height, atmosphereSettings);
    float haze = densityMie(height, atmosphereSettings);
    float3 baseScatter = rayleighScattering(atmosphereSettings) * air * rayleighPhase(nu) +
                         mieScattering(atmosphereSettings) * haze * miePhase(nu, atmosphereSettings.scatteringParams.y);
    float order = max(atmosphereSettings.debugExposureOrder.z, 1.0f);
    float multipleScale = 1.0f + 0.12f * (order - 1.0f);
    float3 radiance = baseScatter * transView * irradiance * 18000.0f * multipleScale;
    scatteringLut[dispatchThreadID] = float4(radiance, haze);
}
