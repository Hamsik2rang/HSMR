module Grid;
import GlobalUniform;

// ForwardGridPass는 binding = 0에 PerViewUBO를 바인딩합니다.
ConstantBuffer<PerViewUBO> perView : register(b0);

// Grid 파라미터. GPU 외부에서 동적으로 전달할 필요가 없어서 코드 안에 const로 세팅합니다.
static const float g_GridSize = 100.0f;
static const float g_GridCellSize = 1.0f;
static const float4 g_GridColorThin = float4(0.5f, 0.5f, 0.5f, 0.5f);
static const float4 g_GridColorThick = float4(0.6f, 0.6f, 0.6f, 1.0f);
static const float g_GridMinPixelsBetweenCells = 2.0f;

struct FSInput
{
    float4 positionCS : SV_Position;
    float2 uv : TEXCOORD0;
    float2 camPos : TEXCOORD1;
};

float max2(float2 v)
{
    return max(v.x, v.y);
}

float mod(float x, float y)
{
    return x - y * floor(x / y);
}

float2 mod(float2 v, float b)
{
    return float2(mod(v.x, b), mod(v.y, b));
}

float4 gridColor(float2 uv, float2 camPos) 
{
    float2 originUV = uv;

    float2 dudv = float2(
        length(float2(ddx(uv.x), ddy(uv.x))),
        length(float2(ddx(uv.y), ddy(uv.y)))
    );

    float lodLevel = max(0.0, log10((length(dudv) * g_GridMinPixelsBetweenCells) / g_GridCellSize) + 1.0);
    float lodFade = frac(lodLevel);

    // LOD에 따른 셀 사이즈 계산
    float lod0 = g_GridCellSize * pow(10.0, floor(lodLevel));
    float lod1 = lod0 * 10.0;
    float lod2 = lod1 * 10.0;

    // 4픽셀 굵기로 안티에일리어싱 적용
    dudv *= 4.0;

    // 알파 계산을 위해 앤티앨리어싱 처리된 선의 중심을 그리드 좌표로 설정
    uv += dudv * 0.5;

    // 각 LOD에 대해 셀 라인 중심까지의 절대 거리를 계산 및 최대 X/Y 값을 선택해서 커버리지 알파 값 계산
    float lod0a = max2(float2(1.0) - abs(saturate(mod(uv, lod0) / dudv) * 2.0 - float2(1.0)));
    float lod1a = max2(float2(1.0) - abs(saturate(mod(uv, lod1) / dudv) * 2.0 - float2(1.0)));
    float lod2a = max2(float2(1.0) - abs(saturate(mod(uv, lod2) / dudv) * 2.0 - float2(1.0)));

    uv -= camPos;

    // LOD 전환 처리를 위해 falloff 색상값 블렌딩
    float4 color = lod2a > 0.0 ? g_GridColorThick : lod1a > 0.0 ? lerp(g_GridColorThick, g_GridColorThin, lodFade) : g_GridColorThin;

    // 그리드 범위까지의 거리 기반으로 Opacity 감소율 계산
    float opacityFalloff = (1.0 - saturate(length(uv) / g_GridSize));

    float minX = min(dudv.x, 1.0f);
    float minZ = min(dudv.y, 1.0f);

    // if(originUV.x > -0.1f * minX && originUV.x < 0.1f * minX) color.z = 1.0f;
    // if(originUV.y > -0.1f * minZ && originUV.y < 0.1f * minZ) color.x = 1.0f;
    
    color.z = lerp(color.z, 1.0f, originUV.x > -0.1f * minX && originUV.x < 0.1f * minX);
    color.x = lerp(color.x, 1.0f, originUV.y > -0.1f * minZ && originUV.y < 0.1f * minZ);

    // LOD 레벨 알파값과 스케일 블렌딩
    color.a *= (lod2a > 0.0 ? lod2a : lod1a > 0.0 ? lod1a : (lod0a * (1.0-lodFade))) * opacityFalloff;

    return color;
}

[shader("vertex")]
FSInput VertexMain(uint vertexIndex : SV_VertexID)
{
    // Fullscreen Quad
    float3 worldPositions[4] =
    {
        float3(-1.0f, 0.0f, -1.0f),
        float3( 1.0f, 0.0f, -1.0f),
        float3(-1.0f, 0.0f,  1.0f),
        float3( 1.0f, 0.0f,  1.0f)
    };
    
    int indices[6] = {0, 1, 2, 2, 1, 3};

    int index = indices[vertexIndex];
    float3 positionWS = worldPositions[index] * g_GridSize;
    positionWS.x += perView.cameraPositionTime.x;
    positionWS.z += perView.cameraPositionTime.z;
    float4 positionCS = mul(perView.viewProjectionMatrix, float4(positionWS, 1.0f));

    FSInput output;
    output.uv = positionWS.xz;
    output.camPos = perView.cameraPositionTime.xz;
    output.positionCS = positionCS;
    
    return output;
}

[shader("fragment")]
float4 FragmentMain(FSInput input): SV_Target
{
    return gridColor(input.uv, input.camPos);
}
