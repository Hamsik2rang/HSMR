module FinalPost;
import GlobalUniform;
import Color;

Texture2D inputTexture;
SamplerState linearSampler;

struct VSInput
{
    float3 positionOS : POSITION0;
    float2 uv         : TEXCOORD0;
};

struct FSInput
{
    float4 positionCS : SV_Position;
    float2 uv         : TEXCOORD0;
};

cbuffer FinalPostConfig
{
    int toneMapperType;
    bool needGammaCorrection;
    int futureUse0;
    int futureUse1;
};

[shader("vertex")]
FSInput VertexMain(VSInput input)
{
    FSInput output;
    output.positionCS = float4(input.positionOS, 1.0f);
    output.uv = input.uv;
    return output;
}

[shader("fragment")]
float4 FragmentMain(FSInput input) : SV_Target
{
    // ... Color Grading, Tone Mapping 등 적용 시 여기에 추가
    

    // Gamma Correction 적용. needGammaCorrection이 true라면 Swapchain의 포맷이 UNORM이므로, 선형 색상을 sRGB로 변환하여 출력해야 함.
    float4 color = inputTexture.Sample(linearSampler, input.uv);
    return lerp(color, LinearToSrgb(color), needGammaCorrection);
}
