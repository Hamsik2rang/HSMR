//
// Sky.vert.slang
// Vertex shader for atmospheric sky rendering
//
// Renders a fullscreen triangle and computes view ray direction
//

// Uniform buffer containing camera and rendering parameters
struct SkyUniforms
{
    float4x4 inverseViewProjection;
    float3   cameraPosition;
    float    _pad0;
    float3   sunDirection;
    float    sunAngularRadius;
    float3   earthCenter;
    float    exposure;
    float3   whitePoint;
    float    _pad1;
};

[[vk::binding(0, 0)]]
ConstantBuffer<SkyUniforms> uniforms;

struct VertexInput
{
    [[vk::location(0)]]
    float3 position : POSITION;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 viewRay  : TEXCOORD0;
    float2 uv       : TEXCOORD1;
};

[shader("vertex")]
VertexOutput VertexMain(VertexInput input)
{
    VertexOutput output;

    // Pass through position (already in clip space for fullscreen triangle)
    output.position = float4(input.position, 1.0);

    // Compute UV coordinates from position
    output.uv = input.position.xy * 0.5 + 0.5;

    // Compute view ray direction in world space
    // We need to transform clip-space position to world-space direction
    float4 clipPos = float4(input.position.xy, 1.0, 1.0);
    float4 worldPos = mul(uniforms.inverseViewProjection, clipPos);
    worldPos.xyz /= worldPos.w;

    // Direction from camera to this point
    output.viewRay = normalize(worldPos.xyz - uniforms.cameraPosition);

    return output;
}
