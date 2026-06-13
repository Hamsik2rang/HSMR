module VolumetricCloud;
import GlobalUniform;

ConstantBuffer<PerViewUBO> perView;

struct CloudSettingsUBO
{
    float4 sunDirectionIntensity;
    float4 sunColorAmbient;
    float4 coverageTypePrecipDensity;
    float4 erosionWindDebugTime;
    float4 layerAndAtmosphere;
    float4 sampleAndLighting;
};

ConstantBuffer<CloudSettingsUBO> cloudSettings;
Sampler3D baseNoiseTexture;
Sampler3D detailNoiseTexture;
Sampler2D curlWeatherTexture;

struct AtmosphereSettingsUBO
{
    float4 planetRadii;
    float4 densityParams;
    float4 scatteringParams;
    float4 sunDirectionIntensity;
    float4 debugExposureOrder;
};

ConstantBuffer<AtmosphereSettingsUBO> atmosphereSettings;
Sampler2D atmosphereTransmittanceLut;
Sampler2D atmosphereIrradianceLut;
Sampler3D atmosphereScatteringLut;

struct VSOutput
{
    float4 positionCS : SV_Position;
    float3 viewDirWS  : TEXCOORD0;
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

float saturate1(float v)
{
    return clamp(v, 0.0f, 1.0f);
}

float remap(float value, float oldMin, float oldMax, float newMin, float newMax)
{
    return newMin + saturate1((value - oldMin) / max(oldMax - oldMin, 0.0001f)) * (newMax - newMin);
}

float heightGradientStratus(float h)
{
    return saturate1(remap(h, 0.02f, 0.18f, 0.0f, 1.0f)) *
           saturate1(remap(h, 0.88f, 1.0f, 1.0f, 0.0f));
}

float heightGradientStratocumulus(float h)
{
    float bottom = saturate1(remap(h, 0.04f, 0.22f, 0.0f, 1.0f));
    float top = saturate1(remap(h, 0.72f, 1.0f, 1.0f, 0.0f));
    float dome = lerp(0.65f, 1.0f, smoothstep(0.15f, 0.55f, h));
    return bottom * top * dome;
}

float heightGradientCumulus(float h)
{
    float bottom = saturate1(remap(h, 0.02f, 0.24f, 0.0f, 1.0f));
    float top = saturate1(remap(h, 0.58f, 1.0f, 1.0f, 0.0f));
    float bulb = smoothstep(0.08f, 0.42f, h) * saturate1(remap(h, 1.0f, 0.45f, 0.0f, 1.0f));
    return bottom * top * lerp(0.45f, 1.0f, bulb);
}

float cloudHeightGradient(float h, float cloudType)
{
    float stratus = heightGradientStratus(h);
    float stratocumulus = heightGradientStratocumulus(h);
    float cumulus = heightGradientCumulus(h);
    float lowBlend = saturate1(cloudType * 2.0f);
    float highBlend = saturate1((cloudType - 0.5f) * 2.0f);
    return lerp(lerp(stratus, stratocumulus, lowBlend), cumulus, highBlend);
}

float2 curlOffset(float2 weatherUv)
{
    float2 curl = curlWeatherTexture.Sample(weatherUv).rg * 2.0f - 1.0f;
    return curl * 0.035f;
}

float sampleBaseDensity(float3 worldPos, float height01)
{
    float time = cloudSettings.erosionWindDebugTime.w;
    float2 wind = cloudSettings.erosionWindDebugTime.yz;
    float2 weatherUv = worldPos.xz * 0.000025f + wind * time * 0.0000018f;
    float weather = curlWeatherTexture.Sample(weatherUv).b;

    float coverage = saturate1(cloudSettings.coverageTypePrecipDensity.x + (weather - 0.5f) * 0.35f);
    float cloudType = saturate1(cloudSettings.coverageTypePrecipDensity.y + (weather - 0.5f) * 0.25f);
    float gradient = cloudHeightGradient(height01, cloudType);

    float3 noiseUv = float3(
        worldPos.xz * 0.000075f + wind * time * 0.000003f,
        worldPos.y * 0.00016f).xzy;
    float4 baseNoise = baseNoiseTexture.Sample(noiseUv);
    float worleyFbm = dot(baseNoise.gba, float3(0.625f, 0.25f, 0.125f));
    float baseShape = remap(baseNoise.r, 1.0f - coverage * 0.82f, 1.0f, 0.0f, 1.0f);
    baseShape = saturate1(baseShape - (1.0f - worleyFbm) * 0.18f);

    float bottomFade = saturate1(remap(height01, 0.0f, 0.13f, 0.0f, 1.0f));
    return baseShape * gradient * bottomFade;
}

float sampleDensity(float3 worldPos, float height01, float detailStrength)
{
    float baseDensity = sampleBaseDensity(worldPos, height01);
    if (baseDensity <= 0.001f)
    {
        return 0.0f;
    }

    float time = cloudSettings.erosionWindDebugTime.w;
    float2 wind = cloudSettings.erosionWindDebugTime.yz;
    float2 weatherUv = worldPos.xz * 0.00008f + wind * time * 0.000004f;
    float2 curl = curlOffset(weatherUv);
    float3 detailUv = float3(
        worldPos.xz * 0.00042f + curl + wind * time * 0.000006f,
        worldPos.y * 0.00055f).xzy;

    float3 detail = detailNoiseTexture.Sample(detailUv).rgb;
    float erosionNoise = dot(detail, float3(0.625f, 0.25f, 0.125f));
    float edgeWeight = 1.0f - saturate1(baseDensity * 1.35f);
    float erosion = cloudSettings.erosionWindDebugTime.x * saturate1(detailStrength);
    float density = baseDensity - (1.0f - erosionNoise) * edgeWeight * erosion;
    density = saturate1(density) * cloudSettings.coverageTypePrecipDensity.w;
    return density;
}

bool raySphere(float3 origin, float3 dir, float3 center, float radius, out float t0, out float t1)
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

float henyeyGreenstein(float cosTheta, float g)
{
    float g2 = g * g;
    return (1.0f - g2) / max(0.0001f, pow(1.0f + g2 - 2.0f * g * cosTheta, 1.5f)) * 0.07957747f;
}

float lightMarch(float3 worldPos, float height01, float baseStep)
{
    float3 sunDir = normalize(cloudSettings.sunDirectionIntensity.xyz);
    uint sampleCount = (uint)clamp(cloudSettings.sampleAndLighting.y, 1.0f, 8.0f);
    float opticalDepth = 0.0f;
    float coneStep = max(baseStep * 2.5f, 80.0f);

    for (uint i = 0; i < sampleCount; ++i)
    {
        float fi = (float)i;
        float dist = coneStep * (fi + 1.0f);
        if (i == sampleCount - 1)
        {
            dist *= 4.0f;
        }

        float3 offset = float3(
            sin(fi * 12.9898f) * 0.7f,
            cos(fi * 7.233f) * 0.45f,
            sin(fi * 4.113f) * 0.7f) * dist * 0.08f;
        float3 p = worldPos + sunDir * dist + offset;
        float h = height01 + (dot(sunDir, float3(0.0f, 1.0f, 0.0f)) * dist) /
                  max(cloudSettings.layerAndAtmosphere.y - cloudSettings.layerAndAtmosphere.x, 1.0f);
        if (h > 0.0f && h < 1.0f)
        {
            opticalDepth += sampleDensity(p, h, 0.25f) * dist * 0.00115f;
        }
    }

    float beer = exp(-opticalDepth);
    float powder = 1.0f - exp(-opticalDepth * cloudSettings.sampleAndLighting.w);
    return saturate1(beer * lerp(0.72f, 1.35f, powder));
}

float2 atmosphereTransmittanceUv(float heightMeters, float mu)
{
    float height01 = saturate1(heightMeters / max(atmosphereSettings.planetRadii.z, 1.0f));
    return float2(saturate1(mu * 0.5f + 0.5f), height01);
}

float3 atmosphereScatteringUv(float3 rayOrigin, float3 rayDir)
{
    float planetRadius = atmosphereSettings.planetRadii.x;
    float3 planetCenter = float3(0.0f, -planetRadius, 0.0f);
    float3 up = normalize(rayOrigin - planetCenter);
    float mu = dot(rayDir, up);
    float sunMu = dot(normalize(atmosphereSettings.sunDirectionIntensity.xyz), up);
    float height = length(rayOrigin - planetCenter) - planetRadius;
    return float3(
        saturate1(mu * 0.5f + 0.5f),
        saturate1(sunMu * 0.5f + 0.5f),
        saturate1(height / max(atmosphereSettings.planetRadii.z, 1.0f)));
}

float3 tonemapAtmosphere(float3 color)
{
    return 1.0f.xxx - exp(-max(color, 0.0f.xxx) * max(atmosphereSettings.debugExposureOrder.y, 0.001f));
}

float3 skyColor(float3 rayOrigin, float3 dir)
{
    float3 uvw = atmosphereScatteringUv(rayOrigin, dir);
    float3 sky = atmosphereScatteringLut.Sample(uvw).rgb;

    float planetRadius = atmosphereSettings.planetRadii.x;
    float3 planetCenter = float3(0.0f, -planetRadius, 0.0f);
    float3 up = normalize(rayOrigin - planetCenter);
    float height = length(rayOrigin - planetCenter) - planetRadius;
    float sunMu = dot(normalize(atmosphereSettings.sunDirectionIntensity.xyz), up);
    sky += atmosphereIrradianceLut.Sample(float2(saturate1(sunMu * 0.5f + 0.5f), saturate1(height / max(atmosphereSettings.planetRadii.z, 1.0f)))).rgb * 0.015f;

    float sunAmount = pow(saturate1(dot(dir, normalize(atmosphereSettings.sunDirectionIntensity.xyz))), 1800.0f);
    float3 sunTrans = atmosphereTransmittanceLut.Sample(atmosphereTransmittanceUv(height, sunMu)).rgb;
    sky += sunTrans * atmosphereSettings.sunDirectionIntensity.w * sunAmount;
    return tonemapAtmosphere(sky);
}

float3 sunTransmittanceAt(float3 worldPos)
{
    float planetRadius = atmosphereSettings.planetRadii.x;
    float3 planetCenter = float3(0.0f, -planetRadius, 0.0f);
    float3 up = normalize(worldPos - planetCenter);
    float height = length(worldPos - planetCenter) - planetRadius;
    float sunMu = dot(normalize(atmosphereSettings.sunDirectionIntensity.xyz), up);
    return atmosphereTransmittanceLut.Sample(atmosphereTransmittanceUv(height, sunMu)).rgb;
}

float3 ambientIrradianceAt(float3 worldPos)
{
    float planetRadius = atmosphereSettings.planetRadii.x;
    float3 planetCenter = float3(0.0f, -planetRadius, 0.0f);
    float3 up = normalize(worldPos - planetCenter);
    float height = length(worldPos - planetCenter) - planetRadius;
    float sunMu = dot(normalize(atmosphereSettings.sunDirectionIntensity.xyz), up);
    return atmosphereIrradianceLut.Sample(float2(saturate1(sunMu * 0.5f + 0.5f), saturate1(height / max(atmosphereSettings.planetRadii.z, 1.0f)))).rgb;
}

[shader("fragment")]
float4 FragmentMain(VSOutput input) : SV_Target
{
    float3 rayOrigin = perView.cameraPositionTime.xyz;
    float3 rayDir = normalize(input.viewDirWS);
    float3 planetCenter = float3(0.0f, -cloudSettings.layerAndAtmosphere.z, 0.0f);
    float planetRadius = cloudSettings.layerAndAtmosphere.z;
    float cloudBottom = planetRadius + cloudSettings.layerAndAtmosphere.x;
    float cloudTop = planetRadius + cloudSettings.layerAndAtmosphere.y;
    float3 localUp = normalize(rayOrigin - planetCenter);
    float horizonDot = dot(rayDir, localUp);

    // The camera starts below the cloud layer. Downward rays are inside both
    // cloud spheres and would otherwise march to the far side of the planet,
    // producing a noisy inverted cloud carpet across the lower screen.
    if (horizonDot < -0.015f)
    {
        return float4(skyColor(rayOrigin, rayDir), 1.0f);
    }

    float outerT0;
    float outerT1;
    if (!raySphere(rayOrigin, rayDir, planetCenter, cloudTop, outerT0, outerT1))
    {
        return float4(skyColor(rayOrigin, rayDir), 1.0f);
    }

    float innerT0;
    float innerT1;
    bool hitInner = raySphere(rayOrigin, rayDir, planetCenter, cloudBottom, innerT0, innerT1);

    float cameraAltitude = length(rayOrigin - planetCenter) - planetRadius;
    float marchStart = max(0.0f, outerT0);
    if (cameraAltitude < cloudSettings.layerAndAtmosphere.x && hitInner)
    {
        marchStart = max(marchStart, innerT1);
    }

    float marchEnd = max(marchStart, outerT1);
    marchEnd = min(marchEnd, marchStart + 35000.0f);
    if (marchEnd <= marchStart)
    {
        return float4(skyColor(rayOrigin, rayDir), 1.0f);
    }

    uint sampleCount = (uint)clamp(cloudSettings.sampleAndLighting.x, 16.0f, 160.0f);
    float stepSize = (marchEnd - marchStart) / (float)sampleCount;
    float t = marchStart;
    float alpha = 0.0f;
    float3 cloudColor = float3(0.0f);
    float densityDebug = 0.0f;
    float lightingDebug = 0.0f;
    uint litSamples = 0;
    uint zeroRun = 0;
    bool fullMarch = false;
    float debugView = cloudSettings.layerAndAtmosphere.w;

    if (debugView == 1.0f)
    {
        float3 p = rayOrigin + rayDir * marchStart;
        float h = 0.5f;
        return float4(baseNoiseTexture.Sample(p * 0.00008f).rrr, 1.0f);
    }
    if (debugView == 2.0f)
    {
        float3 p = rayOrigin + rayDir * marchStart;
        return float4(detailNoiseTexture.Sample(p * 0.00042f).rgb, 1.0f);
    }

    [loop]
    for (uint i = 0; i < sampleCount && alpha < 0.985f; ++i)
    {
        float3 p = rayOrigin + rayDir * t;
        float height = length(p - planetCenter) - planetRadius;
        float height01 = saturate1((height - cloudSettings.layerAndAtmosphere.x) /
                         max(cloudSettings.layerAndAtmosphere.y - cloudSettings.layerAndAtmosphere.x, 1.0f));

        float baseDensity = sampleBaseDensity(p, height01);
        float density = 0.0f;
        float currentStep = stepSize;

        if (!fullMarch && baseDensity > 0.012f)
        {
            fullMarch = true;
            t = max(marchStart, t - stepSize);
            continue;
        }

        if (fullMarch)
        {
            float distance01 = saturate1((t - marchStart) / 35000.0f);
            float horizonDetailFade = smoothstep(0.015f, 0.18f, horizonDot);
            float detailStrength = (1.0f - distance01) * horizonDetailFade;
            density = sampleDensity(p, height01, detailStrength);
            if (density <= 0.001f)
            {
                zeroRun++;
                if (zeroRun > 4)
                {
                    fullMarch = false;
                    zeroRun = 0;
                }
            }
            else
            {
                zeroRun = 0;
            }
        }
        else
        {
            currentStep = stepSize * 2.0f;
        }

        if (density > 0.001f)
        {
            float lightEnergy = lightMarch(p, height01, stepSize);
            float phase = henyeyGreenstein(dot(rayDir, normalize(cloudSettings.sunDirectionIntensity.xyz)),
                                           cloudSettings.sampleAndLighting.z);
            float3 sunTrans = sunTransmittanceAt(p);
            float direct = lightEnergy * phase * cloudSettings.sunDirectionIntensity.w * dot(sunTrans, float3(0.3333f)) * 6.0f;
            float ambient = cloudSettings.sunColorAmbient.w * lerp(0.75f, 1.2f, height01);
            float3 sampleColor = cloudSettings.sunColorAmbient.rgb * direct + tonemapAtmosphere(ambientIrradianceAt(p) * 0.035f) * ambient;

            float sampleAlpha = 1.0f - exp(-density * currentStep * 0.0014f);
            sampleAlpha = saturate1(sampleAlpha);
            cloudColor += (1.0f - alpha) * sampleColor * sampleAlpha;
            alpha += (1.0f - alpha) * sampleAlpha;
            densityDebug += density;
            lightingDebug += lightEnergy;
            litSamples++;
        }

        t += currentStep;
        if (t >= marchEnd)
        {
            break;
        }
    }

    if (debugView == 3.0f)
    {
        return float4(saturate1(densityDebug * 0.08f).xxx, 1.0f);
    }
    if (debugView == 4.0f)
    {
        float lighting = litSamples > 0 ? lightingDebug / (float)litSamples : 0.0f;
        return float4(lighting.xxx, 1.0f);
    }

    float3 sky = skyColor(rayOrigin, rayDir);
    float distanceFade = saturate1((marchEnd - marchStart) / 22000.0f);
    float3 atmosphere = lerp(cloudColor, sky, distanceFade * 0.18f);
    float horizonFade = smoothstep(-0.015f, 0.08f, horizonDot);
    float3 color = lerp(sky, atmosphere, alpha * horizonFade);
    return float4(color, 1.0f);
}
