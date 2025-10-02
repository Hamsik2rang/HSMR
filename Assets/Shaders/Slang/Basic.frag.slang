#include "GlobalInputLayout.hlsli"

[shader("fragment")]
float4 FragmentMain(FSInput input) : SV_Target
{
    return input.color;
}