module AtmosphereSingleScattering;
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
    float nu = lerp(-0.2f, 1.0f, saturate1(viewMu * sunMu * 0.5f + 0.5f));

    float rayleighDensity = densityRayleigh(height, atmosphereSettings);
    float mieDensity = densityMie(height, atmosphereSettings);
    float3 transView = transmittanceFromHeightMu(height, viewMu, atmosphereSettings);
    float3 transSun = transmittanceFromHeightMu(height, sunMu, atmosphereSettings);

    float3 rayleigh = rayleighScattering(atmosphereSettings) * rayleighDensity * rayleighPhase(nu);
    float3 mie = mieScattering(atmosphereSettings) * mieDensity * miePhase(nu, atmosphereSettings.scatteringParams.y);
    float horizonBoost = smoothstep(-0.08f, 0.25f, viewMu);
    float3 radiance = (rayleigh + mie) * transView * transSun * atmosphereSettings.sunDirectionIntensity.w * lerp(12000.0f, 38000.0f, horizonBoost);
    scatteringLut[dispatchThreadID] = float4(radiance, mieDensity);
}
