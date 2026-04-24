module Skybox;
import GlobalUniform;

ConstantBuffer<PerViewUBO> perView;
SamplerCube skyboxCubemap;

struct VSOutput
{
    float4 positionCS : SV_Position;
    float3 viewDirWS  : TEXCOORD0;
};

// Fullscreen triangle: vertexID 0,1,2 covers the entire screen without a vertex buffer.
// z = w so after perspective divide z/w = 1.0 (NDC far plane), placing the skybox
// behind all opaque geometry when depth test is LessOrEqual.
[shader("vertex")]
VSOutput VertexMain(uint vertexID : SV_VertexID)
{
    float2 uv      = float2((vertexID << 1) & 2, vertexID & 2);
    float4 clipPos = float4(uv * 2.0f - 1.0f, 1.0f, 1.0f);

    // Reconstruct world-space view direction: clip -> view (inv proj), view -> world (inv view rotation).
    // w=0 in the final mul so translation is not applied (direction only).
    float4 viewPos = mul(perView.inverseProjectionMatrix, float4(clipPos.xy, 0.0f, 1.0f));
    viewPos /= viewPos.w;
    float3 worldDir = mul(perView.inverseViewMatrix, float4(viewPos.xyz, 0.0f)).xyz;

    VSOutput output;
    output.positionCS = clipPos;
    output.viewDirWS  = worldDir;
    return output;
}

[shader("fragment")]
float4 FragmentMain(VSOutput input) : SV_Target
{
    float3 dir = normalize(input.viewDirWS);
    return skyboxCubemap.Sample(dir);
}
