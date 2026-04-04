module Grid;
import GlobalUniform;

// ForwardGridPass는 binding=0에 PerViewUBO를 바인딩합니다.
ConstantBuffer<PerViewUBO> perView : register(b0);

struct FSInput
{
    float4 positionCS : SV_Position;
    float3 nearPoint  : TEXCOORD0;
    float3 farPoint   : TEXCOORD1;
};

// clip 공간 좌표를 역변환해 월드 공간 레이의 시작/끝 점을 구합니다.
float3 unprojectPoint(float x, float y, float z, float4x4 invVP)
{
    float4 clipSpace    = float4(x, y, z, 1.0);
    float4 worldSpace   = mul(invVP, clipSpace);
    return worldSpace.xyz / worldSpace.w;
}

[shader("vertex")]
FSInput VertexMain(uint vertexIndex : SV_VertexID)
{
    // Fullscreen triangle: clip 좌표를 직접 할당합니다.
    float2 clipPositions[3] =
    {
        float2(-1.0, -1.0),
        float2( 3.0, -1.0),
        float2(-1.0,  3.0),
    };

    float2 pos = clipPositions[vertexIndex];

    FSInput output;
    output.positionCS = float4(pos, 0.0, 1.0);

    // 각 버텍스에서 near/far 월드 공간 레이를 미리 계산합니다.
    float4x4 invVP = perView.inverseViewProjectionMatrix;
    output.nearPoint = unprojectPoint(pos.x, pos.y, 0.0, invVP);
    output.farPoint  = unprojectPoint(pos.x, pos.y, 1.0, invVP);

    return output;
}

// 격자 선 강도를 계산합니다 (Shadertoy "Infinite Grid" 기법).
float4 grid(float3 fragPos, float scale)
{
    float2 coord      = fragPos.xz * scale;
    float2 derivative = fwidth(coord);
    float2 grid2      = abs(frac(coord - 0.5) - 0.5) / derivative;
    float  line2      = min(grid2.x, grid2.y);
    float  minimumz   = min(derivative.y, 1.0);
    float  minimumx   = min(derivative.x, 1.0);
    float4 color      = float4(0.2, 0.2, 0.2, 1.0 - min(line2, 1.0));

    // X=0, Z=0 강조축
    if (fragPos.x > -0.1 * minimumx && fragPos.x < 0.1 * minimumx)
        color.z = 1.0;
    if (fragPos.z > -0.1 * minimumz && fragPos.z < 0.1 * minimumz)
        color.x = 1.0;

    return color;
}

[shader("fragment")]
float4 FragmentMain(FSInput input) : SV_Target
{
    // Y=0 평면과 레이의 교점을 계산합니다.
    float t = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);

    // Y=0 평면 뒤(t<0)나 너무 먼 곳(t>1)은 그리지 않습니다.
    if (t < 0.0 || t > 1.0)
        discard;

    float3 fragPos = input.nearPoint + t * (input.farPoint - input.nearPoint);

    float4 color = grid(fragPos, 1.0) + grid(fragPos, 0.1);

    // 원점에서 멀어질수록 페이드 아웃합니다.
    float dist = length(fragPos.xz);
    float fade = 1.0 - smoothstep(50.0, 100.0, dist);
    color.a   *= fade;

    if (color.a < 0.005)
        discard;

    return color;
}
