#ifndef __GLOBAL_UNIFORM_HLSLI__
#define __GLOBAL_UNIFORM_HLSLI__

struct PerViewUBO
{
    float4x4 view;
    float4x4 projection;
    float4x4 viewprojection;
    float4x4 inverseView;
    float4x4 inverseProjection;
    float4x4 inverseViewProjection;
    
    float2 resolution;
    float time;
    float padding;
};
ConstantBuffer<PerViewUBO> perView;

struct PerDrawUBO
{
    float4x4 model;
    float4x4 inverseModel;
};
ConstantBuffer<PerDrawUBO> perDraw;

#endif