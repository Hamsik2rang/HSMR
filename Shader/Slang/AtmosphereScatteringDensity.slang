module AtmosphereScatteringDensity;
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
    float height = uvw.z * atmosphereSettings.planetRadii.z;
    float air = densityRayleigh(height, atmosphereSettings);
    float haze = densityMie(height, atmosphereSettings);
    float sunLift = saturate1(uvw.y);
    float3 densityRadiance = (rayleighScattering(atmosphereSettings) * air + mieScattering(atmosphereSettings) * haze) *
                             atmosphereSettings.sunDirectionIntensity.w *
                             lerp(8000.0f, 22000.0f, sunLift);
    scatteringLut[dispatchThreadID] = float4(densityRadiance, haze);
}
