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

struct PerDrawUBO
{
    float4x4 modelMatrix;
    float4x4 inverseModelMatrix;
};

ConstantBuffer<PerViewUBO> perView : register(b1);
ConstantBuffer<PerDrawUBO> perDraw : register(b2);

struct VSInput
{
    float3 positionOS : POSITION0;
    float3 normalOS   : NORMAL0;
};

struct FSInput
{
    float4 positionCS  : SV_Position;
    float3 normalWS    : NORMAL0;
    float3 positionWS  : TEXCOORD0;
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

    return output;
}
