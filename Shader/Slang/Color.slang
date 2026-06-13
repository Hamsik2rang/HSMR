module Color;

// Standard sRGB OETF (IEC 61966-2-1 / BT.709 transfer curve). Used here because
// the swapchain is a UNORM format, so the shader must encode the linear
// fragment color into sRGB before writing — otherwise the display reads it as
// already-sRGB and the image looks dim.
public float LinearToSrgb(float c)
{
    return (c <= 0.0031308)
        ? (c * 12.92)
        : (pow(c, 1.0 / 2.4) * 1.055 - 0.055);
}

public float2 LinearToSrgb(float2 c)
{
    return float2(LinearToSrgb(c.x),
                  LinearToSrgb(c.y));
}

public float3 LinearToSrgb(float3 c)
{
    return float3(LinearToSrgb(c.r),
                  LinearToSrgb(c.g),
                  LinearToSrgb(c.b));
}

public float4 LinearToSrgb(float4 c)
{
    return float4(LinearToSrgb(c.r),
                  LinearToSrgb(c.g),
                  LinearToSrgb(c.b),
                  c.a); // Alpha는 보정 대상이 아님.
}

public float SrgbToLinear(float c)
{
    return (c <= 0.04045)
        ? (c / 12.92)
        : pow((c + 0.055) / 1.055, 2.4);
}

public float2 SrgbToLinear(float2 c)
{
    return float2(SrgbToLinear(c.x),
                  SrgbToLinear(c.y));
}

public float3 SrgbToLinear(float3 c)
{
    return float3(SrgbToLinear(c.r),
                  SrgbToLinear(c.g),
                  SrgbToLinear(c.b));
}

public float4 SrgbToLinear(float4 c)
{
    return float4(SrgbToLinear(c.r),
                  SrgbToLinear(c.g),
                  SrgbToLinear(c.b),
                  c.a); // Alpha는 보정 대상이 아님.
}
