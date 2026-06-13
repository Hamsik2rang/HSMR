module PBR;
import GlobalUniform;
import Macros;
import Color;

ConstantBuffer<PerViewUBO> perView;
ConstantBuffer<PerDrawUBO> perDraw;
ConstantBuffer<LightUBO> lightUBO;

struct MaterialParams
{
    float4 diffuseColor;   // baseColorFactor (rgb) + alpha
    float4 emissionColor;  // emissiveFactor (rgb)
    float4 ambientColor;   // ambient tint (rgb)
    float  roughness;      // roughnessFactor
    float  metallic;       // metallicFactor
    float  opacity;
    int    twoSided;
};

ConstantBuffer<MaterialParams> materialParams;

Sampler2D baseColorSampler   : register(s0);
Sampler2D normalSampler      : register(s1);
Sampler2D metallicSampler    : register(s2);
Sampler2D roughnessSampler   : register(s3);
Sampler2D occlusionSampler   : register(s4);
Sampler2D emissiveSampler    : register(s5);

struct VSInput
{
    float3 positionOS : POSITION0;
    float2 uv         : TEXCOORD0;
    float3 normalOS   : NORMAL0;
    float3 tangentOS  : TANGENT0;
};

struct FSInput
{
    float4 positionCS : SV_Position;
    float3 normalWS   : NORMAL0;
    float3 tangentWS  : TANGENT0;
    float3 bitangentWS: TEXCOORD2;
    float3 positionWS : TEXCOORD0;
    float2 uv         : TEXCOORD1;
};

[shader("vertex")]
FSInput VertexMain(VSInput input)
{
    FSInput output;

    float4 worldPos   = mul(perDraw.modelMatrix, float4(input.positionOS, 1.0f));
    output.positionCS = mul(perView.viewProjectionMatrix, worldPos);
    output.positionWS = worldPos.xyz;

    float4x4 im = perDraw.inverseModelMatrix;
    float3x3 normalMatrix = float3x3(
        float3(im[0][0], im[1][0], im[2][0]),
        float3(im[0][1], im[1][1], im[2][1]),
        float3(im[0][2], im[1][2], im[2][2])
    );

    float3 N = normalize(mul(normalMatrix, input.normalOS));
    float3 T = normalize(mul((float3x3)perDraw.modelMatrix, input.tangentOS));
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T);

    output.normalWS    = N;
    output.tangentWS   = T;
    output.bitangentWS = B;
    output.uv          = input.uv;

    return output;
}

float distributionGGX(float NdotH, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denom * denom, EPSILON);
}

float geometrySchlickGGX(float NdotX, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) * 0.125;
    return NdotX / (NdotX * (1.0 - k) + k);
}

float geometrySmith(float NdotL, float NdotV, float roughness)
{
    return geometrySchlickGGX(NdotL, roughness) * geometrySchlickGGX(NdotV, roughness);
}

float3 fresnelSchlick(float HdotV, float3 F0)
{
    float t = 1.0 - HdotV;
    return F0 + (1.0 - F0) * (t * t * t * t * t);
}

[shader("fragment")]
float4 FragmentMain(FSInput input) : SV_Target
{
    float4 baseSample = baseColorSampler.Sample(input.uv) * materialParams.diffuseColor;
    float3 albedo     = baseSample.rgb;

    float metallicSample  = metallicSampler.Sample(input.uv).r;
    float roughnessSample = roughnessSampler.Sample(input.uv).r;
    float aoSample        = occlusionSampler.Sample(input.uv).r;
    float3 emissiveSample = emissiveSampler.Sample(input.uv).rgb * materialParams.emissionColor.rgb;

    float metallic  = saturate(metallicSample * materialParams.metallic);
    float roughness = clamp(roughnessSample * materialParams.roughness, 0.04, 1.0);

    // Normal map → world space via TBN.
    float3 nTex = normalSampler.Sample(input.uv).rgb * 2.0 - 1.0;
    float3 N    = normalize(input.normalWS);
    float3 T    = normalize(input.tangentWS);
    float3 B    = normalize(input.bitangentWS);
    float3 Nws  = normalize(T * nTex.x + B * nTex.y + N * nTex.z);

    float3 V = normalize(perView.cameraPositionTime.xyz - input.positionWS);
    // lightUBO.directionType.xyz is the direction the light travels in world space.
    float3 L = normalize(-lightUBO.directionType.xyz);
    float3 H = normalize(L + V);

    float NdotL = saturate(dot(Nws, L));
    float NdotV = saturate(dot(Nws, V));
    float NdotH = saturate(dot(Nws, H));
    float HdotV = saturate(dot(H, V));

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

    float D  = distributionGGX(NdotH, roughness);
    float G  = geometrySmith(NdotL, NdotV, roughness);
    float3 F = fresnelSchlick(HdotV, F0);

    float3 specular = (D * G * F) / max(4.0 * NdotL * NdotV, EPSILON);
    float3 kD       = (float3(1.0, 1.0, 1.0) - F) * (1.0 - metallic);
    float3 diffuse  = kD * albedo / PI;

    float3 lightColor = lightUBO.colorIntensity.rgb * lightUBO.colorIntensity.w;
    float3 direct     = (diffuse + specular) * lightColor * NdotL;

    float3 ambient = albedo * materialParams.ambientColor.rgb * aoSample;
    float3 color   = direct + ambient + emissiveSample;

    // Swapchain is UNORM; encode to sRGB so the display sees the right values.
    float3 colorSrgb = LinearToSrgb(max(color, 0.0));

    return float4(colorSrgb, baseSample.a * materialParams.opacity);
}
