module EditorIcon;
import GlobalUniform;

ConstantBuffer<PerViewUBO> perView;
Sampler2D cameraIconSampler;
Sampler2D lightIconSampler;

struct VSInput
{
    float2 localPosition : POSITION0;
    float2 uv : TEXCOORD0;
    float4 worldPositionAndSize : TEXCOORD1;
    float4 tintAndAlpha : COLOR0;
    float4 iconMeta : TEXCOORD2;
};

struct FSInput
{
    float4 positionCS : SV_Position;
    float2 uv : TEXCOORD0;
    float4 tintAndAlpha : COLOR0;
    float iconType : TEXCOORD1;
};

[shader("vertex")]
FSInput VertexMain(VSInput input)
{
    FSInput output;

    float3 worldPosition = input.worldPositionAndSize.xyz;
    float worldSize = input.worldPositionAndSize.w;
    float iconType = input.iconMeta.x;
    float3 cameraRight = normalize(mul(perView.inverseViewMatrix, float4(1.0, 0.0, 0.0, 0.0)).xyz);
    float3 cameraUp = normalize(mul(perView.inverseViewMatrix, float4(0.0, 1.0, 0.0, 0.0)).xyz);
    float3 billboardOffset = (cameraRight * input.localPosition.x + cameraUp * input.localPosition.y) * worldSize;
    float3 billboardWorldPosition = worldPosition + billboardOffset;

    output.positionCS = mul(perView.viewProjectionMatrix, float4(billboardWorldPosition, 1.0));
    output.uv = input.uv;
    output.tintAndAlpha = input.tintAndAlpha;
    output.iconType = iconType;
    return output;
}

[shader("fragment")]
float4 FragmentMain(FSInput input) : SV_Target
{
    float4 sampled = input.iconType > 0.5
        ? lightIconSampler.Sample(input.uv)
        : cameraIconSampler.Sample(input.uv);

    float alpha = sampled.a * input.tintAndAlpha.a;
    return float4(sampled.rgb * input.tintAndAlpha.rgb, alpha);
}
