#ifndef __GLOBAL_INPUT_LAYOUT_HLSLI__
#define __GLOBAL_INPUT_LAYOUT_HLSLI__

struct VSInput
{
    float3 positionOS : POSITION0;
    float4 color : COLOR0;
};

struct FSInput
{
    float4 positionCS : SV_Position;
    float4 color : COLOR0;
};

Texture2D _Albedo;
SamplerState sampler_Albedo;

#endif