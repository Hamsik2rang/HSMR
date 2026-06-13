module AtmosphereCommon;

public static const uint TRANSMITTANCE_WIDTH = 256;
public static const uint TRANSMITTANCE_HEIGHT = 64;
public static const uint IRRADIANCE_WIDTH = 64;
public static const uint IRRADIANCE_HEIGHT = 16;
public static const uint SCATTERING_WIDTH = 256;
public static const uint SCATTERING_HEIGHT = 128;
public static const uint SCATTERING_DEPTH = 32;

public struct AtmosphereSettingsUBO
{
    public float4 planetRadii;
    public float4 densityParams;
    public float4 scatteringParams;
    public float4 sunDirectionIntensity;
    public float4 debugExposureOrder;
};

public float saturate1(float value)
{
    return clamp(value, 0.0f, 1.0f);
}

public float3 saturate3(float3 value)
{
    return clamp(value, 0.0f.xxx, 1.0f.xxx);
}

public float safeSqrt(float value)
{
    return sqrt(max(value, 0.0f));
}

public float remap01(float value, float oldMin, float oldMax)
{
    return saturate1((value - oldMin) / max(oldMax - oldMin, 0.00001f));
}

public float rayleighPhase(float nu)
{
    return 0.0596831f * (1.0f + nu * nu);
}

public float miePhase(float nu, float g)
{
    float g2 = g * g;
    return 0.07957747f * (1.0f - g2) / max(0.0001f, pow(1.0f + g2 - 2.0f * g * nu, 1.5f));
}

public float3 rayleighScattering(AtmosphereSettingsUBO settings)
{
    return float3(5.802f, 13.558f, 33.100f) * 0.000001f * settings.densityParams.z;
}

public float3 mieScattering(AtmosphereSettingsUBO settings)
{
    return float3(3.996f, 3.996f, 3.996f) * 0.000001f * settings.densityParams.w;
}

public float3 ozoneAbsorption(AtmosphereSettingsUBO settings)
{
    return float3(0.650f, 1.881f, 0.085f) * 0.000001f * settings.scatteringParams.x;
}

public float densityRayleigh(float heightMeters, AtmosphereSettingsUBO settings)
{
    return exp(-max(heightMeters, 0.0f) / max(settings.densityParams.x, 1.0f));
}

public float densityMie(float heightMeters, AtmosphereSettingsUBO settings)
{
    return exp(-max(heightMeters, 0.0f) / max(settings.densityParams.y, 1.0f));
}

public bool raySphere(float3 origin, float3 dir, float3 center, float radius, out float t0, out float t1)
{
    float3 oc = origin - center;
    float b = dot(oc, dir);
    float c = dot(oc, oc) - radius * radius;
    float h = b * b - c;
    if (h < 0.0f)
    {
        t0 = 0.0f;
        t1 = 0.0f;
        return false;
    }
    h = sqrt(h);
    t0 = -b - h;
    t1 = -b + h;
    return t1 > 0.0f;
}

public float3 opticalDepthToAtmosphereTop(float heightMeters, float mu, AtmosphereSettingsUBO settings)
{
    float planetRadius = settings.planetRadii.x;
    float topRadius = settings.planetRadii.y;
    float3 planetCenter = float3(0.0f, -planetRadius, 0.0f);
    float3 origin = float3(0.0f, heightMeters, 0.0f);
    float sinTheta = safeSqrt(1.0f - mu * mu);
    float3 dir = normalize(float3(sinTheta, mu, 0.0f));

    float t0;
    float t1;
    if (!raySphere(origin, dir, planetCenter, topRadius, t0, t1))
    {
        return 900000.0f.xxx;
    }

    float rayLength = max(t1, 0.0f);
    float stepSize = rayLength / 32.0f;
    float rayleighDepth = 0.0f;
    float mieDepth = 0.0f;
    float ozoneDepth = 0.0f;

    for (uint i = 0; i < 32; ++i)
    {
        float t = (float(i) + 0.5f) * stepSize;
        float3 p = origin + dir * t;
        float h = max(length(p - planetCenter) - planetRadius, 0.0f);
        rayleighDepth += densityRayleigh(h, settings) * stepSize;
        mieDepth += densityMie(h, settings) * stepSize;
        float ozoneLayer = exp(-abs(h - 25000.0f) / 15000.0f);
        ozoneDepth += ozoneLayer * stepSize;
    }

    return rayleighScattering(settings) * rayleighDepth +
           mieScattering(settings) * 1.1f * mieDepth +
           ozoneAbsorption(settings) * ozoneDepth;
}

public float3 transmittanceFromHeightMu(float heightMeters, float mu, AtmosphereSettingsUBO settings)
{
    return exp(-opticalDepthToAtmosphereTop(heightMeters, mu, settings));
}

public float2 transmittanceUvFromHeightMu(float heightMeters, float mu, AtmosphereSettingsUBO settings)
{
    float height01 = saturate1(heightMeters / max(settings.planetRadii.z, 1.0f));
    return float2(saturate1(mu * 0.5f + 0.5f), height01);
}

public float3 scatteringUvFromWorldDir(float3 cameraPos, float3 rayDir, AtmosphereSettingsUBO settings)
{
    float planetRadius = settings.planetRadii.x;
    float atmosphereHeight = settings.planetRadii.z;
    float3 planetCenter = float3(0.0f, -planetRadius, 0.0f);
    float3 up = normalize(cameraPos - planetCenter);
    float mu = dot(rayDir, up);
    float sunMu = dot(normalize(settings.sunDirectionIntensity.xyz), up);
    float height = length(cameraPos - planetCenter) - planetRadius;
    return float3(
        saturate1(mu * 0.5f + 0.5f),
        saturate1(sunMu * 0.5f + 0.5f),
        saturate1(height / max(atmosphereHeight, 1.0f)));
}

public float3 tonemapAtmosphere(float3 color, AtmosphereSettingsUBO settings)
{
    return 1.0f.xxx - exp(-max(color, 0.0f.xxx) * max(settings.debugExposureOrder.y, 0.001f));
}
