# Precomputed Atmospheric Scattering 구현 계획

> **기반 논문**: Eric Bruneton & Fabrice Neyret, "Precomputed Atmospheric Scattering", Eurographics Symposium on Rendering 2008
> **참조 구현체**: [ebruneton/precomputed_atmospheric_scattering](https://github.com/ebruneton/precomputed_atmospheric_scattering) (2017년 개선 버전)

---

## 1. 개요

### 1.1 목표
HSMR 렌더링 엔진에 물리 기반 대기 산란 시스템을 구현하여 다음 효과를 실시간으로 렌더링:
- 하늘 색상 (주간/황혼/일출)
- 공기 원근법 (Aerial Perspective)
- 다중 산란 (Multiple Scattering)
- 빛 기둥 (Light Shafts / God Rays) - 선택적

### 1.2 기술적 특징
- **Rayleigh 산란**: 공기 분자에 의한 산란 (파란 하늘)
- **Mie 산란**: 에어로졸 입자에 의한 산란 (태양 주변 후광)
- **흡수층 지원**: 오존층 등 흡수 매체 (참조 구현체 추가 기능)
- **사전계산 기반**: LUT(Look-Up Table)를 사용한 실시간 렌더링
- **모든 시점 지원**: 지상 ~ 우주 연속적 렌더링

---

## 2. 참조 구현체 분석 결과

### 2.1 핵심 발견 사항

참조 구현체(`precomputed_atmospheric_scattering`)는 2008년 논문의 개선된 버전으로, 다음과 같은 특징이 있음:

| 항목 | 2008 논문 | 2017 참조 구현체 |
|------|----------|-----------------|
| **흡수층** | 미지원 | Ozone 등 지원 |
| **밀도 프로파일** | 단일 지수함수 | 2-레이어 지원 |
| **파장 모드** | Radiance 전용 | Radiance + Illuminance |
| **Mie 텍스처** | 통합 | 분리/통합 선택 가능 |
| **3D 텍스처 렌더링** | 미명시 | Geometry Shader + MRT (→ Compute Shader로 대체 예정) |

### 2.2 텍스처 사양 (참조 구현체 기준)

```cpp
// constants.h에서 발췌
constexpr int TRANSMITTANCE_TEXTURE_WIDTH = 256;
constexpr int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

constexpr int SCATTERING_TEXTURE_R_SIZE = 32;      // altitude
constexpr int SCATTERING_TEXTURE_MU_SIZE = 128;    // view zenith
constexpr int SCATTERING_TEXTURE_MU_S_SIZE = 32;   // sun zenith
constexpr int SCATTERING_TEXTURE_NU_SIZE = 8;      // view-sun angle

// 4D → 3D 패킹: (NU × MU_S) × MU × R = 256 × 128 × 32
constexpr int SCATTERING_TEXTURE_WIDTH = 256;   // 8 × 32
constexpr int SCATTERING_TEXTURE_HEIGHT = 128;
constexpr int SCATTERING_TEXTURE_DEPTH = 32;

constexpr int IRRADIANCE_TEXTURE_WIDTH = 64;
constexpr int IRRADIANCE_TEXTURE_HEIGHT = 16;
```

### 2.3 사전계산 방식

#### 참조 구현체 방식 (OpenGL)
참조 구현체는 **Fragment Shader + Geometry Shader + MRT** 방식 사용:
- Geometry Shader로 `gl_Layer` 설정하여 3D 텍스처 slice 지정
- Fragment Shader에서 실제 계산 수행
- MRT로 여러 텍스처 동시 출력

#### HSMR 채택 방식 (Compute Shader)
Geometry Shader 대신 **Compute Shader**를 사용하여 더 현대적이고 효율적으로 구현:

```
┌─────────────────────────────────────────────────────────────┐
│  3D 텍스처 렌더링 방식 (Compute Shader)                       │
├─────────────────────────────────────────────────────────────┤
│  1. 3D Dispatch (width/8, height/8, depth)                  │
│  2. 각 스레드가 SV_DispatchThreadID로 3D 좌표 획득            │
│  3. 해당 좌표에서 산란/투과율 계산                            │
│  4. RWTexture3D/RWTexture2D에 imageStore로 직접 쓰기         │
│  5. 누적이 필요한 경우 imageLoad + imageStore 조합            │
└─────────────────────────────────────────────────────────────┘
```

**Compute Shader 장점:**
- 래스터라이제이션 파이프라인 오버헤드 없음
- 3D 인덱싱이 자연스러움
- Metal/Vulkan 모두 완벽 지원
- Geometry Shader 의존성 제거

### 2.4 API 함수 명세

참조 구현체가 제공하는 셰이더 API:

```glsl
// 하늘 렌더링 - 카메라에서 대기 경계까지
vec3 GetSkyRadiance(
    vec3 camera,           // 행성 중심 기준 카메라 위치
    vec3 view_ray,         // 시선 방향 (단위벡터)
    float shadow_length,   // 그림자 영역 길이 (light shaft용)
    vec3 sun_direction,    // 태양 방향 (단위벡터)
    out vec3 transmittance // 출력: 투과율
);

// Aerial Perspective - 카메라에서 특정 점까지
vec3 GetSkyRadianceToPoint(
    vec3 camera,
    vec3 point,            // 대상 점 위치
    float shadow_length,
    vec3 sun_direction,
    out vec3 transmittance
);

// 지표면 조사량 - 태양 + 하늘 기여
vec3 GetSunAndSkyIrradiance(
    vec3 p,                // 지표면 위치
    vec3 normal,           // 표면 노멀
    vec3 sun_direction,
    out vec3 sky_irradiance // 출력: 하늘 기여분
);

// 태양 radiance (대기 밖)
vec3 GetSolarRadiance();
```

### 2.5 Radiance vs Illuminance 모드

| 모드 | 조건 | 특징 |
|------|------|------|
| **Radiance** | `num_wavelengths ≤ 3` | 3파장(R,G,B) 직접 저장, 빠름 |
| **Illuminance** | `num_wavelengths > 3` | 다파장 적분 → sRGB 변환, 정확함 |

---

## 3. 현재 엔진 상태 분석

### 3.1 지원되는 기능 ✅
| 기능 | 상태 | 위치 |
|------|------|------|
| RHI 추상화 | 완성 | `Source/RHI/` |
| Metal/Vulkan 백엔드 | 완성 | `Source/RHI/Metal/`, `Source/RHI/Vulkan/` |
| 셰이더 컴파일 (HLSL→SPIRV/MSL) | 완성 | `Source/ShaderSystem/` |
| 2D 텍스처 | 완성 | `RHIDefinition.h` |
| Uniform/Storage 버퍼 | 완성 | `RHIDefinition.h` |
| 다중 렌더 패스 | 완성 | `Source/Renderer/` |

### 3.2 추가 구현 필요 ⚠️
| 기능 | 현재 상태 | 필요 작업 | 우선순위 |
|------|----------|----------|----------|
| **3D 텍스처** | 미지원 | `ETextureType::TEX_3D` 추가 | 🔴 필수 |
| **Compute Shader** | 부분 구현 | 타입 정의됨, Command 구현 필요 | 🔴 필수 |
| **RWTexture (UAV)** | 미확인 | Compute Shader 출력용 | 🔴 필수 |

---

## 4. 필요 리소스 명세

### 4.1 사전계산 텍스처

| 이름 | 차원 | 해상도 | 포맷 | 용도 | 영구/임시 |
|------|------|--------|------|------|----------|
| **transmittance_texture** | 2D | 256 × 64 | RGBA32F* | 투과율 T(r, μ) | 영구 |
| **scattering_texture** | 3D | 256 × 128 × 32 | RGBA16F | 산란 S | 영구 |
| **irradiance_texture** | 2D | 64 × 16 | RGBA32F | 지표면 조사량 E | 영구 |
| **single_mie_scattering** | 3D | 256 × 128 × 32 | RGB16F | Mie 단일 산란 (선택) | 영구 |
| **delta_irradiance** | 2D | 64 × 16 | RGBA32F | 중간 계산 | 임시 |
| **delta_rayleigh_scattering** | 3D | 256 × 128 × 32 | RGB16F | 중간 계산 | 임시 |
| **delta_mie_scattering** | 3D | 256 × 128 × 32 | RGB16F | 중간 계산 | 임시 |
| **delta_scattering_density** | 3D | 256 × 128 × 32 | RGB16F | 중간 계산 | 임시 |

*참고: Transmittance는 32F 필수 (16F는 아티팩트 발생)*

### 4.2 대기 파라미터 구조체 (참조 구현체 기준)

```cpp
// 밀도 프로파일 레이어 (최대 2개)
struct DensityProfileLayer {
    float width;          // 레이어 두께 (m), 마지막 레이어는 무시
    float exp_term;       // 지수 계수 (무차원)
    float exp_scale;      // 지수 스케일 (m^-1)
    float linear_term;    // 선형 계수 (m^-1)
    float constant_term;  // 상수항 (무차원)
    // density = exp_term * exp(exp_scale * h) + linear_term * h + constant_term
};

struct DensityProfile {
    DensityProfileLayer layers[2];
};

struct AtmosphereParameters {
    // 태양 파라미터
    vec3 solar_irradiance;      // 대기 상단 태양 조사량 (W/m²/nm)
    float sun_angular_radius;   // 태양 각반경 (rad), < 0.1 권장

    // 행성 파라미터
    float bottom_radius;        // 지표면 반경 (m), 6360 km
    float top_radius;           // 대기 상단 반경 (m), 6420 km

    // Rayleigh 산란
    DensityProfile rayleigh_density;
    vec3 rayleigh_scattering;   // 산란 계수 @ 해수면 (m^-1)

    // Mie 산란
    DensityProfile mie_density;
    vec3 mie_scattering;        // 산란 계수 @ 해수면 (m^-1)
    vec3 mie_extinction;        // 소멸 계수 @ 해수면 (m^-1)
    float mie_phase_function_g; // 비대칭 인자 (0.76~0.9)

    // 흡수층 (Ozone 등) - 논문에 없던 기능
    DensityProfile absorption_density;
    vec3 absorption_extinction; // 소멸 계수 (m^-1)

    // 지표면
    vec3 ground_albedo;         // 평균 반사율
    float mu_s_min;             // cos(최대 태양 천정각)
};
```

### 4.3 지구 대기 기본값

```cpp
// 참조 구현체의 demo에서 사용하는 값
const float kBottomRadius = 6360000.0;  // m
const float kTopRadius = 6420000.0;     // m

// Rayleigh
const vec3 kRayleighScattering = vec3(5.802e-6, 13.558e-6, 33.1e-6);  // m^-1
const float kRayleighScaleHeight = 8000.0;  // m

// Mie
const vec3 kMieScattering = vec3(3.996e-6);  // m^-1
const vec3 kMieExtinction = vec3(4.440e-6);  // m^-1
const float kMieScaleHeight = 1200.0;  // m
const float kMiePhaseG = 0.8;

// Ozone (25km 고도 중심, ±15km)
const vec3 kOzoneExtinction = vec3(0.650e-6, 1.881e-6, 0.085e-6);  // m^-1
```

---

## 5. 구현 단계

### Phase 1: RHI 확장 (필수 선행 작업)

#### 5.1.1 3D 텍스처 타입 추가
**파일**: `Source/RHI/RHIDefinition.h`
```cpp
enum class ETextureType {
    INVALID = 0,
    TEX_1D,
    TEX_1D_ARRAY,
    TEX_2D,
    TEX_2D_ARRAY,
    TEX_CUBE,
    TEX_3D,         // 추가
};
```

#### 5.1.2 Metal 구현
**파일**: `Source/RHI/Metal/MetalContext.mm`
```objc
// 텍스처 타입 매핑
case ETextureType::TEX_3D:
    descriptor.textureType = MTLTextureType3D;
    break;
```

#### 5.1.3 Vulkan 구현
**파일**: `Source/RHI/Vulkan/VulkanContext.cpp`
```cpp
// 이미지 타입 매핑
case ETextureType::TEX_3D:
    imageInfo.imageType = VK_IMAGE_TYPE_3D;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    break;
```

#### 5.1.4 Compute Shader 지원 완성

**Metal 구현** (`Source/RHI/Metal/`):
```objc
// Compute Pipeline 생성
id<MTLComputePipelineState> computePipeline =
    [device newComputePipelineStateWithFunction:kernelFunction error:&error];

// Dispatch
[computeEncoder dispatchThreadgroups:threadgroupsPerGrid
               threadsPerThreadgroup:threadsPerThreadgroup];
```

**Vulkan 구현** (`Source/RHI/Vulkan/`):
```cpp
// Compute Pipeline 생성
VkComputePipelineCreateInfo pipelineInfo{};
pipelineInfo.stage = computeShaderStageInfo;
vkCreateComputePipelines(device, cache, 1, &pipelineInfo, nullptr, &pipeline);

// Dispatch
vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
```

#### 5.1.5 RWTexture (UAV) 지원

3D 텍스처에 Compute Shader에서 쓰기 위한 Unordered Access View 지원:

```hlsl
// HLSL Compute Shader
RWTexture3D<float4> OutputScattering : register(u0);
RWTexture2D<float4> OutputTransmittance : register(u1);

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID) {
    // 직접 쓰기
    OutputScattering[id] = ComputeScattering(id);
}
```

**Metal**: `MTLTextureUsageShaderWrite` 플래그 설정
**Vulkan**: `VK_IMAGE_USAGE_STORAGE_BIT` 플래그 설정

#### 5.1.6 예상 작업량
- **난이도**: 중상
- **예상 시간**: 3~4일
- **세부 작업**:
  - 3D 텍스처 타입 추가: 0.5일
  - Compute Pipeline 생성/바인딩: 1~1.5일
  - RWTexture (UAV) 바인딩: 1일
  - 테스트 및 디버깅: 0.5~1일
- **테스트**: 3D 텍스처 생성 → Compute Shader 쓰기 → 샘플링 검증

---

### Phase 2: 사전계산 파이프라인

#### 5.2.1 사전계산 Compute Shader

| Compute Shader | 입력 (SRV) | 출력 (UAV) | Dispatch 크기 |
|----------------|------------|------------|---------------|
| `CS_Transmittance` | - | transmittance | (256/8, 64/8, 1) |
| `CS_DirectIrradiance` | T | delta_irradiance, irradiance | (64/8, 16/8, 1) |
| `CS_SingleScattering` | T | delta_rayleigh, delta_mie, scattering | (256/8, 128/8, 32) |
| `CS_ScatteringDensity` | T, deltaR, deltaM, multiS, E | scattering_density | (256/8, 128/8, 32) |
| `CS_IndirectIrradiance` | deltaR, deltaM, multiS | delta_irradiance, irradiance | (64/8, 16/8, 1) |
| `CS_MultipleScattering` | T, scattering_density | delta_multiple, scattering | (256/8, 128/8, 32) |

**Compute Shader 예시:**
```hlsl
// CS_SingleScattering.hlsl
Texture2D<float4> TransmittanceTexture : register(t0);
RWTexture3D<float4> DeltaRayleighScattering : register(u0);
RWTexture3D<float4> DeltaMieScattering : register(u1);
RWTexture3D<float4> Scattering : register(u2);

[numthreads(8, 8, 1)]
void CSMain(uint3 id : SV_DispatchThreadID) {
    float3 uvw = (id + 0.5) / float3(256, 128, 32);

    // 4D → 3D 언패킹
    float4 params = GetScatteringParameters(uvw);

    // 단일 산란 계산
    float3 rayleigh, mie;
    ComputeSingleScattering(params, rayleigh, mie);

    // UAV에 직접 쓰기
    DeltaRayleighScattering[id] = float4(rayleigh, 0);
    DeltaMieScattering[id] = float4(mie, 0);

    // 누적 (기존 값 + 새 값)
    Scattering[id] += float4(rayleigh, mie.r);
}
```

#### 5.2.2 사전계산 알고리즘 (Compute Shader 버전)

```
┌─────────────────────────────────────────────────────────────┐
│  사전계산 알고리즘 (Compute Shader)                           │
├─────────────────────────────────────────────────────────────┤
│  1. Dispatch CS_Transmittance (32×8×1 groups)               │
│     → transmittance_texture                                 │
│                                                             │
│  2. Dispatch CS_DirectIrradiance (8×2×1 groups)             │
│     → delta_irradiance, irradiance (초기화)                  │
│                                                             │
│  3. Dispatch CS_SingleScattering (32×16×32 groups)          │
│     → delta_rayleigh, delta_mie                             │
│     → scattering (누적), single_mie (누적)                   │
│     ※ 단일 Dispatch로 전체 3D 텍스처 처리                     │
│                                                             │
│  4. for order in [2, num_scattering_orders]:                │
│       a. Dispatch CS_ScatteringDensity (32×16×32 groups)    │
│          → delta_scattering_density                         │
│                                                             │
│       b. Dispatch CS_IndirectIrradiance (8×2×1 groups)      │
│          → delta_irradiance, irradiance (누적)               │
│                                                             │
│       c. Dispatch CS_MultipleScattering (32×16×32 groups)   │
│          → delta_multiple, scattering (누적)                 │
│                                                             │
│  최종 결과: transmittance, irradiance, scattering           │
└─────────────────────────────────────────────────────────────┘
```

**참조 구현체 대비 장점:**
- 레이어별 루프 불필요 (단일 Dispatch로 전체 3D 처리)
- Geometry Shader 의존성 제거
- GPU 점유율 향상 (더 많은 스레드 동시 실행)

#### 5.2.3 핵심 GLSL 함수 (functions.glsl에서)

```glsl
// 투과율 계산
DimensionlessSpectrum ComputeTransmittanceToTopAtmosphereBoundary(
    IN(AtmosphereParameters) atmosphere,
    Length r, Number mu);

// 단일 산란 계산
void ComputeSingleScattering(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    Length r, Number mu, Number mu_s, Number nu,
    bool ray_r_mu_intersects_ground,
    OUT(IrradianceSpectrum) rayleigh,
    OUT(IrradianceSpectrum) mie);

// 산란 밀도 계산
RadianceDensitySpectrum ComputeScatteringDensity(
    IN(AtmosphereParameters) atmosphere,
    IN(TransmittanceTexture) transmittance_texture,
    IN(ReducedScatteringTexture) single_rayleigh_scattering_texture,
    IN(ReducedScatteringTexture) single_mie_scattering_texture,
    IN(ScatteringTexture) multiple_scattering_texture,
    IN(IrradianceTexture) irradiance_texture,
    Length r, Number mu, Number mu_s, Number nu,
    int scattering_order);
```

#### 5.2.4 예상 작업량
- **난이도**: 상
- **예상 시간**: 4~6일
- **핵심 난점**:
  - 정확한 파라미터화 구현
  - MRT 지원 필요
  - Additive blending 설정

---

### Phase 3: 런타임 렌더링

#### 5.3.1 렌더링 API 구현

```cpp
class AtmosphereRenderer {
public:
    // 초기화 - 사전계산 수행
    void Init(const AtmosphereParameters& params,
              int num_scattering_orders = 4);

    // 셰이더 uniform 바인딩
    void SetProgramUniforms(
        RHIShader* shader,
        int transmittance_unit,
        int scattering_unit,
        int irradiance_unit,
        int single_mie_unit = -1);

private:
    RHITexture* transmittance_texture_;
    RHITexture* scattering_texture_;
    RHITexture* irradiance_texture_;
    RHITexture* optional_single_mie_scattering_texture_;
};
```

#### 5.3.2 렌더링 셰이더 사용 예

```hlsl
// Sky.hlsl
#include "AtmosphereAPI.hlsl"

float3 RenderSky(float3 camera, float3 view_ray, float3 sun_direction) {
    float3 transmittance;
    float3 radiance = GetSkyRadiance(
        camera - earth_center,
        view_ray,
        0.0,  // shadow_length
        sun_direction,
        transmittance);

    // 태양 디스크 렌더링
    if (dot(view_ray, sun_direction) > cos(sun_angular_radius)) {
        radiance += transmittance * GetSolarRadiance();
    }

    return radiance;
}

// 지형에 aerial perspective 적용
float3 ApplyAerialPerspective(
    float3 camera, float3 point,
    float3 surface_radiance, float3 sun_direction) {

    float3 transmittance;
    float3 in_scatter = GetSkyRadianceToPoint(
        camera - earth_center,
        point - earth_center,
        0.0,
        sun_direction,
        transmittance);

    return surface_radiance * transmittance + in_scatter;
}
```

#### 5.3.3 톤 매핑

```hlsl
// 참조 구현체의 톤 매핑
float3 ToneMap(float3 radiance, float exposure, float3 white_point) {
    return pow(1.0 - exp(-radiance / white_point * exposure), 1.0 / 2.2);
}
```

#### 5.3.4 예상 작업량
- **난이도**: 중
- **예상 시간**: 2~3일

---

### Phase 4: Light Shafts (선택적)

#### 5.4.1 참조 구현체 방식

```glsl
// shadow_length 파라미터 활용
void GetSphereShadowInOut(
    vec3 view_direction, vec3 sun_direction,
    out float d_in, out float d_out);

// 사용 예
float shadow_length = max(0.0, shadow_out - shadow_in);
vec3 radiance = GetSkyRadiance(camera, view_ray, shadow_length, sun_dir, trans);
```

참조 구현체는 그림자 볼륨의 시작/끝 거리를 계산하여 `shadow_length` 파라미터로 전달.

#### 5.4.2 예상 작업량
- **난이도**: 상
- **예상 시간**: 3~4일

---

## 6. 파일 구조 계획

```
Source/
├── Atmosphere/                       # 새 모듈
│   ├── CMakeLists.txt
│   ├── Atmosphere.h                  # 통합 헤더
│   ├── AtmosphereConstants.h         # 텍스처 크기 상수
│   ├── AtmosphereParameters.h        # 대기 파라미터 구조체
│   ├── AtmosphereModel.h             # Model 클래스
│   ├── AtmosphereModel.cpp
│   └── Private/
│       └── AtmosphereMath.h          # 수학 유틸리티
│
Shader/
└── Atmosphere/
    ├── Definitions.hlsl              # 타입 정의
    ├── Functions.hlsl                # 핵심 계산 함수
    ├── Precompute/                   # Compute Shaders
    │   ├── CS_Transmittance.hlsl
    │   ├── CS_DirectIrradiance.hlsl
    │   ├── CS_SingleScattering.hlsl
    │   ├── CS_ScatteringDensity.hlsl
    │   ├── CS_IndirectIrradiance.hlsl
    │   └── CS_MultipleScattering.hlsl
    └── Render/                       # Pixel Shaders
        ├── AtmosphereAPI.hlsl        # 렌더링 API
        ├── Sky.hlsl
        └── AerialPerspective.hlsl
```

---

## 7. 테스트 계획

### 7.1 단위 테스트
| 테스트 | 검증 내용 |
|--------|----------|
| 3D 텍스처 생성 | Metal/Vulkan 모두 정상 생성 |
| Compute Shader Dispatch | 올바른 threadgroup 크기 및 실행 |
| RWTexture3D 쓰기 | Compute Shader에서 3D 텍스처 쓰기 |
| 3D 텍스처 샘플링 | trilinear 보간 정확성 |
| UAV Barrier | Dispatch 간 동기화 정확성 |

### 7.2 수치 검증
| 항목 | 검증 방법 |
|------|----------|
| Transmittance | 참조 구현체 출력과 비교 |
| Single scattering | 해수면 + 정오 기준 비교 |
| Multiple scattering | 황혼 장면에서 차이 확인 |

### 7.3 시각적 검증
| 장면 | 비교 대상 |
|------|----------|
| 정오 하늘 | 참조 구현체 demo |
| 일몰/일출 | 논문 Figure 8, 9 |
| 우주에서 본 지구 | 참조 구현체 demo |
| Aerial perspective | 논문 Figure 9 하단 |

### 7.4 성능 목표
| 항목 | 목표 | 참조 구현체 |
|------|------|-----------|
| 사전계산 시간 | < 10초 | ~5초 (GTX 8800) |
| 렌더링 오버헤드 | < 2ms @ 1080p | < 3ms |
| 메모리 사용량 | < 20MB | ~8MB |

---

## 8. 일정 추정

| Phase | 작업 | 예상 기간 | 비고 |
|-------|------|----------|------|
| **1** | RHI 확장 (3D 텍스처 + Compute Shader + UAV) | 3~4일 | 핵심 선행 작업 |
| **2** | 사전계산 파이프라인 | 4~5일 | Compute Shader로 단순화됨 |
| **3** | 런타임 렌더링 | 2~3일 | 비교적 쉬움 |
| **4** | Light Shafts (선택) | 3~4일 | |
| | **통합 및 디버깅** | 2~3일 | |
| | **총계** | **14~19일** | |

**Compute Shader 방식의 이점:**
- Phase 2에서 Geometry Shader + MRT 복잡성 제거로 1일 단축 예상
- 디버깅이 더 직관적 (3D 좌표 기반)

---

## 9. 참고 자료

### 9.1 핵심 자료
- **2008 논문**: [Precomputed Atmospheric Scattering (PDF)](https://hal.inria.fr/inria-00288758/document)
- **참조 구현체**: [GitHub - ebruneton/precomputed_atmospheric_scattering](https://github.com/ebruneton/precomputed_atmospheric_scattering)
- **해설 문서**: 참조 구현체 내 HTML 문서 (`atmosphere/functions.glsl.html`)

### 9.2 보조 자료
- O'Neil, S. (2005). *Accurate Atmospheric Scattering*. GPU Gems 2.
- Hillaire, S. (2020). *A Scalable and Production Ready Sky and Atmosphere Rendering Technique*. EGSR 2020.

---

## 10. 리스크 및 대응

| 리스크 | 영향 | 대응 방안 |
|--------|------|----------|
| Compute Shader 미완성 | 높음 | 우선순위 높게 RHI 확장 작업 |
| RWTexture3D 미지원 | 높음 | UAV 바인딩 로직 추가 구현 |
| UAV Barrier 누락 | 중간 | Dispatch 간 메모리 배리어 필수 삽입 |
| 수치 정밀도 문제 | 중간 | Transmittance는 32F 필수 |
| 크로스 플랫폼 차이 | 중간 | Metal/Vulkan 별도 테스트 |
| Threadgroup 크기 제한 | 낮음 | 플랫폼별 최대 크기 확인 후 조정 |

---

## 11. 주요 변경 사항 (보강)

### 2026-01-02 (2차): Compute Shader 기반 전환
- Geometry Shader + MRT 방식 → **Compute Shader 방식**으로 전환
- Phase 1에 Compute Shader 지원 완성 및 RWTexture(UAV) 지원 추가
- 사전계산 셰이더 테이블을 Compute Shader 기반으로 재작성
- 단위 테스트 항목 업데이트 (Compute Dispatch, UAV Barrier 등)
- 리스크 테이블 업데이트 (Geometry Shader 관련 항목 제거)

### 2026-01-02 (1차): 참조 구현체 분석 반영
- 섹션 2 추가: 참조 구현체 분석 결과
- 텍스처 사양 정정 (참조 구현체 기준)
- AtmosphereParameters 구조체 확장 (DensityProfileLayer, 흡수층)
- 사전계산 알고리즘 상세화
- API 함수 명세 추가
- Radiance/Illuminance 모드 설명 추가
- 일정 추정 조정 (13~19일)

---

---

## 12. 구현 진행 상황

### ✅ Phase 1: RHI 확장 (완료)

#### 1.1 3D 텍스처 타입 추가 ✅
- **커밋**: `6b5c927` feat(RHI): Add 3D texture support for atmospheric scattering
- `ETextureType::TEX_3D` 추가
- Metal: `MTLTextureType3D` 지원
- Vulkan: `VK_IMAGE_TYPE_3D`, `VK_IMAGE_VIEW_TYPE_3D` 지원

#### 1.2 Compute Shader 지원 완성 ✅
- **커밋**: `d85b943` feat(RHI): Add compute pipeline support for Metal and Vulkan
- `RHIComputePipeline` 추상 클래스 추가
- `ComputePipelineInfo` 구조체 추가
- Metal: `MTLComputePipelineState`, `dispatchThreadgroups` 지원
- Vulkan: `VkComputePipeline`, `vkCmdDispatch` 지원
- `RHICommandBuffer::BindComputePipeline`, `Dispatch` 메서드 추가

#### 1.3 RWTexture (UAV) 지원 추가 ✅
- **커밋**: `012b255` feat(RHI): Add UAV (RWTexture) and memory barrier support
- `ETextureUsage::STORAGE` 플래그 추가
- Metal: `MTLTextureUsageShaderWrite` 지원
- Vulkan: `VK_IMAGE_USAGE_STORAGE_BIT` 지원
- `RHICommandBuffer::TextureBarrier` 메서드 추가

#### 1.4 Floating-Point 픽셀 포맷 추가 ✅
- **커밋**: `0cfbdd6` feat(Atmosphere): Add AtmospherePrecompute class and floating-point pixel formats
- `EPixelFormat`: R16F, RG16F, RGBA16F, R32F, RG32F, RGBA32F 추가
- Metal: `MTLPixelFormatR16Float`, `MTLPixelFormatRGBA16Float`, `MTLPixelFormatRGBA32Float` 등 지원

---

### ✅ Phase 2: 사전계산 파이프라인 (완료)

#### 2.1 AtmosphereParameters 구조체 정의 ✅
- **커밋**: `d94bd66` feat(Atmosphere): Add AtmosphereParameters and precomputation shaders
- **파일**: `Source/Engine/Renderer/Atmosphere/AtmosphereParameters.h`
- 구조체:
  - `DensityProfileLayer`: 밀도 프로파일 레이어 (지수/선형 조합)
  - `DensityProfile`: 2레이어 지원 (Rayleigh, Mie, Ozone)
  - `AtmosphereParameters`: 대기 파라미터 전체 (태양, 행성, 산란, 흡수)
- 네임스페이스:
  - `AtmosphereConstants`: 텍스처 크기 상수
  - `EarthAtmosphere`: 지구 대기 기본값 및 `CreateDefault()` 함수

#### 2.2 사전계산 Compute Shader 작성 ✅
- **커밋**: `d94bd66` feat(Atmosphere): Add AtmosphereParameters and precomputation shaders
- **경로**: `Shader/Slang/Atmosphere/`
- 공통 함수:
  - `AtmosphereCommon.hlsli`: 좌표 변환, 위상 함수, 광학 깊이 등
- Compute Shaders (6개):
  - `Transmittance.comp.slang`: 투과율 LUT 계산
  - `DirectIrradiance.comp.slang`: 직접 조사량 계산
  - `SingleScattering.comp.slang`: 단일 산란 계산
  - `ScatteringDensity.comp.slang`: 산란 밀도 계산
  - `IndirectIrradiance.comp.slang`: 간접 조사량 계산
  - `MultipleScattering.comp.slang`: 다중 산란 계산

#### 2.3 AtmospherePrecompute 클래스 구현 ✅
- **커밋**: `0cfbdd6` feat(Atmosphere): Add AtmospherePrecompute class and floating-point pixel formats
- **파일**:
  - `Source/Engine/Renderer/Atmosphere/AtmospherePrecompute.h`
  - `Source/Engine/Renderer/Atmosphere/Private/AtmospherePrecompute.cpp`
- 기능:
  - LUT 텍스처 생성/관리 (9개: transmittance, scattering, irradiance + delta 텍스처들)
  - Compute Shader 로딩 및 파이프라인 생성
  - Resource Layout/Set 생성
  - `Compute()` 메서드로 전체 사전계산 실행
  - 다중 산란 반복 계산 (기본 4차)

---

### ⏳ Phase 3: 런타임 렌더링 (미완료)

#### 남은 작업:
1. **AtmosphereRenderer 클래스 구현**
   - 사전계산 결과를 렌더링에 사용
   - LUT 텍스처 바인딩 관리

2. **렌더링 셰이더 작성**
   - `AtmosphereAPI.hlsli`: GetSkyRadiance, GetSkyRadianceToPoint, GetSunAndSkyIrradiance
   - `Sky.hlsl`: 하늘 렌더링
   - `AerialPerspective.hlsl`: 공기 원근법 적용

3. **ForwardPath/RenderPath 통합**
   - 기존 렌더링 파이프라인에 대기 산란 적용
   - 톤 매핑 및 후처리

4. **테스트 및 검증**
   - 참조 구현체와 비교
   - 시각적 품질 확인

---

### Phase 4: Light Shafts (선택적, 미완료)

- God Rays 효과
- shadow_length 파라미터 활용

---

## 13. 커밋 히스토리

| 커밋 | 설명 | 날짜 |
|------|------|------|
| `0cfbdd6` | feat(Atmosphere): Add AtmospherePrecompute class and floating-point pixel formats | 2026-01-02 |
| `d94bd66` | feat(Atmosphere): Add AtmosphereParameters and precomputation shaders | 2026-01-02 |
| `012b255` | feat(RHI): Add UAV (RWTexture) and memory barrier support | 2026-01-02 |
| `4d87ec9` | docs: Add atmospheric scattering implementation progress document | 2026-01-02 |
| `d85b943` | feat(RHI): Add compute pipeline support for Metal and Vulkan | 2026-01-02 |
| `6b5c927` | feat(RHI): Add 3D texture support for atmospheric scattering | 2026-01-02 |

---

## 14. 파일 구조 (현재)

```
Source/Engine/Renderer/Atmosphere/
├── AtmosphereParameters.h          # 대기 파라미터 구조체
├── AtmospherePrecompute.h          # 사전계산 클래스 헤더
└── Private/
    └── AtmospherePrecompute.cpp    # 사전계산 클래스 구현

Source/RHI/
├── RHIDefinition.h                 # EPixelFormat에 floating-point 포맷 추가
└── Metal/Private/
    └── MetalUtility.mm             # Metal 포맷 변환 함수

Shader/Slang/Atmosphere/
├── AtmosphereCommon.hlsli          # 공통 함수
├── Transmittance.comp.slang        # 투과율 계산
├── DirectIrradiance.comp.slang     # 직접 조사량
├── SingleScattering.comp.slang     # 단일 산란
├── ScatteringDensity.comp.slang    # 산란 밀도
├── IndirectIrradiance.comp.slang   # 간접 조사량
└── MultipleScattering.comp.slang   # 다중 산란
```

---

*문서 작성일: 2026-01-02*
*최종 수정일: 2026-01-02*
*HSMR 엔진 버전: rnd/precomputed-atmospheric-scattering branch*
