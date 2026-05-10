module BlinnPhong;
import GlobalUniform;

ConstantBuffer<PerViewUBO> perView;
ConstantBuffer<PerDrawUBO> perDraw;
ConstantBuffer<LightUBO> lightUBO;

struct MaterialParams
{
    float4 diffuseColor;
    float4 specularColor;
    float4 emissionColor;
    float4 ambientColor;
    float shininess;
    float opacity;
};

ConstantBuffer<MaterialParams> materialParams;
Sampler2D albedoSampler : register(s0);

struct VSInput
{
    float3 positionOS : POSITION0;
    float2 uv  : TEXCOORD0;
    float3 normalOS   : NORMAL0;
};

struct FSInput
{
    float4 positionCS  : SV_Position;
    float3 normalWS    : NORMAL0;
    float3 positionWS  : TEXCOORD0;
    float2 uv    : TEXCOORD1;
};

[shader("vertex")]
FSInput VertexMain(VSInput input)
{
    FSInput output;

    float4 worldPos = mul(perDraw.modelMatrix, float4(input.positionOS, 1.0f));
    output.positionCS = mul(perView.viewProjectionMatrix, worldPos);
    output.positionWS = worldPos.xyz;

    // transpose(inverse(model)) = transpose(inverseModelMatrix)
    // Manual transpose to avoid Slang's Metal vertex stage limitation
    float4x4 im = perDraw.inverseModelMatrix;
    float3x3 normalMatrix = float3x3(
        float3(im[0][0], im[1][0], im[2][0]),
        float3(im[0][1], im[1][1], im[2][1]),
        float3(im[0][2], im[1][2], im[2][2])
    );
    output.normalWS = normalize(mul(normalMatrix, input.normalOS));
    output.uv = input.uv;

    return output;
}

[shader("fragment")]
float4 FragmentMain(FSInput input) : SV_Target
{
    float3 N = normalize(input.normalWS);
    // lightUBO.directionType.xyz is authored as the direction light travels.
    float3 L = normalize(-lightUBO.directionType.xyz);
    float3 V = normalize(perView.cameraPositionTime.xyz - input.positionWS);
    float3 H = normalize(L + V);

    float3 lightColor  = lightUBO.colorIntensity.rgb * lightUBO.colorIntensity.w;

    float ambient  = saturate(materialParams.ambientColor.r);
    float diffuse  = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), max(materialParams.shininess, 1.0));

    float4 albedoSample = albedoSampler.Sample(input.uv) * materialParams.diffuseColor;
    float3 albedo = albedoSample.rgb;
    float3 specularTint = materialParams.specularColor.rgb;
    float3 emission = materialParams.emissionColor.rgb;
    float3 color = albedo * (ambient + diffuse) * lightColor + specular * lightColor * specularTint + emission;

    return float4(color, albedoSample.a * materialParams.opacity);
}
