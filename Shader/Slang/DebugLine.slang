module DebugLine;
import GlobalUniform;

ConstantBuffer<PerViewUBO> perView : register(b0);

struct VSInput
{
    float3 positionWS : POSITION0;
    float4 color : COLOR0;
};

struct FSInput
{
    float4 positionCS : SV_Position;
    float4 color : COLOR0;
};

[shader("vertex")]
FSInput VertexMain(VSInput input)
{
    FSInput output;
    output.positionCS = mul(perView.viewProjectionMatrix, float4(input.positionWS, 1.0f));
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 FragmentMain(FSInput input) : SV_Target
{
    return input.color;
}
