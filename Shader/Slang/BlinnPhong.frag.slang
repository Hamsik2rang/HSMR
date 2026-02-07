struct PerViewUBO
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 viewProjectionMatrix;
    float4x4 inverseViewMatrix;
    float4x4 inverseProjectionMatrix;
    float4x4 inverseViewProjectionMatrix;

    float4 cameraPosition;
};

ConstantBuffer<PerViewUBO> perView : register(b1);

struct FSInput
{
    float4 positionCS  : SV_Position;
    float3 normalWS    : NORMAL0;
    float3 positionWS  : TEXCOORD0;
};

[shader("fragment")]
float4 FragmentMain(FSInput input) : SV_Target
{
    float3 N = normalize(input.normalWS);
    float3 L = normalize(float3(1.0, 1.0, 0.5));
    float3 V = normalize(perView.cameraPosition.xyz - input.positionWS);
    float3 H = normalize(L + V);

    float3 objectColor = float3(0.8, 0.8, 0.8);
    float3 lightColor  = float3(1.0, 0.98, 0.95);

    float ambient  = 0.15;
    float diffuse  = max(dot(N, L), 0.0);
    float specular = pow(max(dot(N, H), 0.0), 64.0);

    float3 color = objectColor * (ambient + diffuse) * lightColor + specular * lightColor * 0.5;

    return float4(color, 1.0);
}
