module GlobalUniform;

// Packed to 6x mat4 + 2x vec4 so C++/Vulkan/Metal layouts agree byte-for-byte.
//   cameraPositionTime : xyz = cameraPos, w = time
//   resolution         : xy  = resolution, zw = padding
public struct PerViewUBO
{
    public float4x4 viewMatrix;
    public float4x4 projectionMatrix;
    public float4x4 viewProjectionMatrix;
    public float4x4 inverseViewMatrix;
    public float4x4 inverseProjectionMatrix;
    public float4x4 inverseViewProjectionMatrix;
    public float4 cameraPositionTime;
    public float4 resolution;
};

public struct PerDrawUBO
{
    public float4x4 modelMatrix;
    public float4x4 inverseModelMatrix;
};

// Packed to 3x vec4 so C++/Vulkan/Metal layouts agree byte-for-byte.
//   position       : xyz = world position, w = padding
//   colorIntensity : rgb = color,          w = intensity
//   directionType  : xyz = travel dir,     w = type stored as float
public struct LightUBO
{
    public float4 position;
    public float4 colorIntensity;
    public float4 directionType;
};

