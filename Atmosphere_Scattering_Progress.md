# Precomputed Atmospheric Scattering - 진행 상황

## 개요
이 문서는 HSMR 엔진에 Precomputed Atmospheric Scattering 기능을 구현하기 위한 작업 진행 상황을 기록합니다.

**브랜치**: `rnd/precomputed-atmospheric-scattering`
**시작일**: 2026-01-02
**최종 업데이트**: 2026-01-02

---

## 완료된 작업

### Phase 1: RHI 확장

#### 1. 3D 텍스처 지원 추가 ✅
**커밋**: `feat(RHI): Add 3D texture support for atmospheric scattering`

- `ETextureType::TEX_3D` 열거형 추가
- Metal: `MTLTextureType3D` 매핑 추가
- Vulkan: `VK_IMAGE_TYPE_3D`, `VK_IMAGE_VIEW_TYPE_3D` 매핑 추가
- Vulkan 버그 수정: `TEX_CUBE`가 `VK_IMAGE_TYPE_3D`로 잘못 매핑되던 문제 수정

**수정된 파일:**
- `Source/RHI/RHIDefinition.h`
- `Source/RHI/Metal/Private/MetalUtility.mm`
- `Source/RHI/Vulkan/Private/VulkanUtility.cpp`

#### 2. Compute Pipeline 지원 추가 ✅
**커밋**: `feat(RHI): Add compute pipeline support for Metal and Vulkan`

##### RHI 공통
- `ComputePipelineInfo` 구조체 구현 (`computeShader`, `resourceLayout`)
- `RHIContext`에 `CreateComputePipeline`/`DestroyComputePipeline` 추가
- `RHICommandBuffer`에 compute 명령 추가:
  - `BindComputePipeline()`
  - `BindComputeResourceSet()`
  - `Dispatch()`

##### Metal 백엔드
- `MetalComputePipeline` 구조체 구현 (`id<MTLComputePipelineState>`)
- `MetalContext::CreateComputePipeline` 구현
- `MetalCommandBuffer` compute 인코더 관리:
  - `id<MTLComputeCommandEncoder> curComputeEncoder`
  - `MetalComputePipeline* curBindComputePipeline`
- 리소스 바인딩 (버퍼, 텍스처, 샘플러)

##### Vulkan 백엔드
- `ComputePipelineVulkan` 구조체 활용
- `createComputePipeline()` 메서드 구현
- `VkComputePipelineCreateInfo` 사용
- `vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE)` 구현
- `vkCmdDispatch()` 구현

**수정된 파일:**
- `Source/RHI/RHIDefinition.h`
- `Source/RHI/RHIContext.h`
- `Source/RHI/CommandHandle.h`
- `Source/RHI/Metal/MetalContext.h`
- `Source/RHI/Metal/MetalRenderHandle.h`
- `Source/RHI/Metal/MetalCommandHandle.h`
- `Source/RHI/Metal/Private/MetalContext.mm`
- `Source/RHI/Metal/Private/MetalRenderHandle.mm`
- `Source/RHI/Metal/Private/MetalCommandHandle.mm`
- `Source/RHI/Vulkan/VulkanContext.h`
- `Source/RHI/Vulkan/VulkanCommandHandle.h`
- `Source/RHI/Vulkan/Private/VulkanContext.cpp`
- `Source/RHI/Vulkan/Private/VulkanCommandHandle.cpp`

---

## 남은 작업

### Phase 1: RHI 확장 (계속)

#### 3. RWTexture (UAV) 지원 추가 ⏳
Compute Shader에서 3D 텍스처에 쓰기 위한 기능
- `ETextureUsage`에 `STORAGE` 플래그 추가
- Metal: `MTLTextureUsageShaderWrite` 적용
- Vulkan: `VK_IMAGE_USAGE_STORAGE_BIT` 적용
- Image memory barrier 처리

### Phase 2: 사전계산 파이프라인 구현 ⏳
- 대기 산란 파라미터 정의 (`AtmosphereParams` 구조체)
- Transmittance LUT 계산 compute shader
- Single Scattering LUT 계산 compute shader
- Multiple Scattering LUT 계산 compute shader
- `AtmospherePrecompute` 클래스 구현

### Phase 3: 런타임 렌더링 구현 ⏳
- Sky rendering fragment shader
- Aerial Perspective 적용
- Sun disc 렌더링
- 렌더링 시스템 통합

---

## 기술적 결정사항

### Geometry Shader 대신 Compute Shader 사용
원본 논문과 레퍼런스 구현에서는 Geometry Shader + MRT를 사용하지만,
본 구현에서는 Compute Shader를 사용하기로 결정함:

**장점:**
1. 엔진이 Geometry Shader를 지원하지 않음
2. Compute Shader가 GPU 활용도가 더 높음
3. 3D 텍스처 쓰기가 더 직관적
4. 모든 백엔드에서 동일한 접근 방식 사용 가능

**접근 방식:**
- 각 3D 텍스처 슬라이스를 별도의 dispatch로 계산
- `Dispatch(width/8, height/8, depth)` 형태로 호출
- threadgroup 크기는 8x8x1 권장

---

## 참고 자료
- 구현 계획서: `Atmosphere_Scattering_Implementation_Plan.md`
- 원본 논문: `Precomputed_Atmosphere_Scattering.pdf`
- 레퍼런스 코드: `/Users/yongsikim/Desktop/Dev/precomputed_atmospheric_scattering`

---

## Git 로그

```
43bcefb feat(RHI): Add compute pipeline support for Metal and Vulkan
[이전 커밋] feat(RHI): Add 3D texture support for atmospheric scattering
```

---

## 다음 작업 시 참고사항

1. **RWTexture 구현 시작점**: `Source/RHI/RHIDefinition.h`의 `ETextureUsage` 열거형
2. **Metal UAV**: `MTLTextureUsageShaderWrite` 플래그 추가 필요
3. **Vulkan UAV**: `VK_IMAGE_USAGE_STORAGE_BIT` 및 descriptor type `VK_DESCRIPTOR_TYPE_STORAGE_IMAGE`
4. **빌드 확인**: macOS에서만 빌드 테스트됨. Windows Vulkan 백엔드 테스트 필요
