#include "GlobalInputLayout.hlsli"

[shader("vertex")]
FSInput VertexMain(VSInput input)
{
    FSInput output;
    output.positionCS = vector<float, 4>(input.positionOS, 1.0f);
    // output.uv = input.uv;
    output.color = input.color;

    return output;
}

[shader("fragment")]
float4 FragmentMain(FSInput input) : SV_Target
{
    return input.color;
}
