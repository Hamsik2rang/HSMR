module GlobalUniform;

public struct PerViewUBO
{
    public float4x4 viewMatrix;
    public float4x4 projectionMatrix;
    public float4x4 viewProjectionMatrix;
    public float4x4 inverseViewMatrix;
    public float4x4 inverseProjectionMatrix;
    public float4x4 inverseViewProjectionMatrix;
    public float4 cameraPositionAndTime;
    public float4 resolutionAndPadding;
};

public struct PerDrawUBO
{
    public float4x4 modelMatrix;
    public float4x4 inverseModelMatrix;
};

public struct LightUBO
{
    public float4 position;
    public float4 colorAndIntensity;
    public float4 directionAndType;
};

