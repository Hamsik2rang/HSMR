module AtmosphereSky;
import GlobalUniform;
import AtmosphereCommon;

ConstantBuffer<PerViewUBO> perView;
ConstantBuffer<AtmosphereSettingsUBO> atmosphereSettings;
Sampler2D transmittanceLut;
Sampler2D irradianceLut;
Sampler3D scatteringLut;

struct VSOutput
{
    float4 positionCS : SV_Position;
    float3 viewDirWS : TEXCOORD0;
};

[shader("vertex")]
VSOutput VertexMain(uint vertexID : SV_VertexID)
{
    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    float4 clipPos = float4(uv * 2.0f - 1.0f, 1.0f, 1.0f);

    float4 viewPos = mul(perView.inverseProjectionMatrix, float4(clipPos.xy, 0.0f, 1.0f));
    viewPos /= viewPos.w;
    float3 worldDir = mul(perView.inverseViewMatrix, float4(viewPos.xyz, 0.0f)).xyz;

    VSOutput output;
    output.positionCS = clipPos;
    output.viewDirWS = worldDir;
    return output;
}

float3 sampleAtmosphereSky(float3 rayOrigin, float3 rayDir)
{
    float3 uvw = scatteringUvFromWorldDir(rayOrigin, rayDir, atmosphereSettings);
    float3 sky = scatteringLut.Sample(uvw).rgb;

    float planetRadius = atmosphereSettings.planetRadii.x;
    float3 planetCenter = float3(0.0f, -planetRadius, 0.0f);
    float3 up = normalize(rayOrigin - planetCenter);
    float sunMu = dot(normalize(atmosphereSettings.sunDirectionIntensity.xyz), up);
    float height = length(rayOrigin - planetCenter) - planetRadius;
    float3 irradiance = irradianceLut.Sample(float2(saturate1(sunMu * 0.5f + 0.5f), saturate1(height / atmosphereSettings.planetRadii.z))).rgb;
    sky += irradiance * 0.015f;

    float sunAmount = pow(saturate1(dot(rayDir, normalize(atmosphereSettings.sunDirectionIntensity.xyz))), 1800.0f);
    float3 sunTrans = transmittanceLut.Sample(transmittanceUvFromHeightMu(height, sunMu, atmosphereSettings)).rgb;
    sky += sunTrans * atmosphereSettings.sunDirectionIntensity.w * sunAmount;
    return tonemapAtmosphere(sky, atmosphereSettings);
}

[shader("fragment")]
float4 FragmentMain(VSOutput input) : SV_Target
{
    float3 rayOrigin = perView.cameraPositionTime.xyz;
    float3 rayDir = normalize(input.viewDirWS);

    float debugView = atmosphereSettings.debugExposureOrder.x;
    if (debugView == 1.0f)
    {
        float planetRadius = atmosphereSettings.planetRadii.x;
        float3 planetCenter = float3(0.0f, -planetRadius, 0.0f);
        float3 up = normalize(rayOrigin - planetCenter);
        float height = length(rayOrigin - planetCenter) - planetRadius;
        float mu = dot(rayDir, up);
        return float4(transmittanceLut.Sample(transmittanceUvFromHeightMu(height, mu, atmosphereSettings)).rgb, 1.0f);
    }
    if (debugView == 2.0f)
    {
        float3 sunDir = normalize(atmosphereSettings.sunDirectionIntensity.xyz);
        float planetRadius = atmosphereSettings.planetRadii.x;
        float3 planetCenter = float3(0.0f, -planetRadius, 0.0f);
        float3 up = normalize(rayOrigin - planetCenter);
        float height = length(rayOrigin - planetCenter) - planetRadius;
        return float4(tonemapAtmosphere(irradianceLut.Sample(float2(saturate1(dot(sunDir, up) * 0.5f + 0.5f), saturate1(height / atmosphereSettings.planetRadii.z))).rgb, atmosphereSettings), 1.0f);
    }
    if (debugView == 3.0f)
    {
        return float4(tonemapAtmosphere(scatteringLut.Sample(scatteringUvFromWorldDir(rayOrigin, rayDir, atmosphereSettings)).rgb, atmosphereSettings), 1.0f);
    }

    return float4(sampleAtmosphereSky(rayOrigin, rayDir), 1.0f);
}
