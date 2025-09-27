#include "GlobalInputLayout.hlsli"

[shader("fragment")]
float4 main(FSInput input) : SV_Target
{
    return input.color;
}