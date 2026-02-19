module BlinnPhong;
import GlobalUniform;

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
    float3 L = normalize(float3(1.0, 1.0, 0.5));
    float3 V = normalize(perView.cameraPosition.xyz - input.positionWS);
    float3 H = normalize(L + V);

    float3 lightColor  = float3(1.0, 0.98, 0.95);

    float ambient  = 0.15;
    float diffuse  = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 64.0);

    float3 albedo = albedoSampler.Sample(input.uv).rgb;
    float3 color = albedo * (ambient + diffuse) * lightColor + specular * lightColor * 0.5;

    return float4(color, 1.0);
}
