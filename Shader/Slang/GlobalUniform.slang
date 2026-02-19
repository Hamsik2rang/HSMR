module GlobalUniform;

public struct PerViewUBO
{
    public float4x4 viewMatrix;
    public float4x4 projectionMatrix;
    public float4x4 viewProjectionMatrix;
    public float4x4 inverseViewMatrix;
    public float4x4 inverseProjectionMatrix;
    public float4x4 inverseViewProjectionMatrix;
    public float4 cameraPosition;
    public float2 resolution;
    public float time;
    public float padding;
};
public ConstantBuffer<PerViewUBO> perView;

public struct PerDrawUBO
{
    public float4x4 modelMatrix;
    public float4x4 inverseModelMatrix;
};
public ConstantBuffer<PerDrawUBO> perDraw;

public struct LightUBO
{
    public float4 position;
    public float3 color;
    public float intensity;
    public float3 direction;
    public int type;
};