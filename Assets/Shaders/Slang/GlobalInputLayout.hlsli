#ifndef __GLOBAL_INPUT_LAYOUT_HLSLI__
#define __GLOBAL_INPUT_LAYOUT_HLSLI__

public struct VSInput
{
    float3 positionOS : POSITION0;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

public struct FSInput
{
    float4 positionCS : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

Texture2D _Albedo;
SamplerState sampler_Albedo;

#endif