module GlobalUniform;

public struct PerViewUBO
{
    public float4x4 viewMatrix;
    public float4x4 projectionMatrix;
    public float4x4 viewProjectionMatrix;
    public float4x4 inverseViewMatrix;
    public float4x4 inverseProjectionMatrix;
    public float4x4 inverseViewProjectionMatrix;
    public float3 cameraPos;
    public float time;
    public float2 resolution;
};

public struct PerDrawUBO
{
    public float4x4 modelMatrix;
    public float4x4 inverseModelMatrix;
};

public struct LightUBO
{
    public float4 position;
    public float3 color;
    public float intensity;
    public float3 direction;
    public int type;
};

