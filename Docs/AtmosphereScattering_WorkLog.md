# Atmospheric Scattering Implementation Work Log

## Overview
Bruneton & Neyret 논문 기반의 Precomputed Atmospheric Scattering 구현 작업 기록.

## Latest Status (Reverted)
스피어와 카메라 회전 기능은 제거됨. 대기 렌더링 기본 코드는 유지.
- 카메라 회전 코드 제거 (AtmosphereSkyPass.h/cpp)
- 하드코딩 구체 제거 (Sky.frag.slang)
- 마우스 입력 처리 코드 제거 (ScenePanel)
- 행렬 transpose 제거 (updateUniforms)
- EditorWindow의 AtmosphereRenderer/AtmosphereSkyPass 코드는 유지됨

## Current Branch
`rnd/precomputed-atmospheric-scattering`

## Completed Work

### 1. Vulkan Buffer Update 구현
- **문제**: `CommandBufferVulkan::UpdateBuffer()`가 비어있어서 uniform buffer 업데이트가 안됨
- **해결**: `vkCmdUpdateBuffer` 사용하여 구현
- **추가 수정**: `VulkanContext.cpp`에서 MAPPED/DYNAMIC 버퍼에 `VK_BUFFER_USAGE_TRANSFER_DST_BIT` 플래그 추가

**파일**: `Source/RHI/Vulkan/Private/VulkanCommandHandle.cpp`
```cpp
void CommandBufferVulkan::UpdateBuffer(RHIBuffer* buffer, const size_t dstOffset, const void* srcData, const size_t dataSize)
{
    HS_ASSERT(buffer, "Buffer is nullptr");
    HS_ASSERT(srcData, "Source data is nullptr");
    HS_ASSERT(dataSize > 0, "Data size must be greater than 0");
    HS_ASSERT(dataSize <= 65536, "vkCmdUpdateBuffer is limited to 65536 bytes");
    BufferVulkan* bufferVK = static_cast<BufferVulkan*>(buffer);
    vkCmdUpdateBuffer(handle, bufferVK->handle, static_cast<VkDeviceSize>(dstOffset), static_cast<VkDeviceSize>(dataSize), srcData);
}
```

### 2. 카메라 좌표계 수정
- **문제**: `camera = cameraPosition - earthCenter` 계산 결과가 대기권 밖 (12,720,100m)
- **해결**:
  - `_cameraPosition = (0, 100, 0)` (월드 좌표, 지면에서 100m)
  - `earthCenter = (0, -6360000, 0)` (지구 중심)
  - 결과: `camera = (0, 6360100, 0)` (지구 중심 기준, 대기권 안)

### 3. GetSkyRadiance 부호 오류 수정
- **문제**: `distanceToTop` 계산에서 부호 오류로 radiance가 0
- **해결**: `-rmu - sqrt(...)` → `-rmu + sqrt(...)`

**파일**: `Shader/Slang/Atmosphere/AtmosphereAPI.hlsli`

### 4. Vulkan Y축 플립
- **문제**: 이미지가 상하 반전됨
- **해결**: `_projectionMatrix[1][1] *= -1.0f`

### 5. 카메라 회전 시스템 추가
- yaw/pitch 기반 카메라 회전 구현
- ScenePanel에서 마우스 우클릭 드래그로 회전

**파일들**:
- `Source/Engine/Renderer/Atmosphere/AtmosphereSkyPass.h` - RotateCamera(), UpdateCameraFromRotation() 추가
- `Source/Engine/Renderer/Atmosphere/Private/AtmosphereSkyPass.cpp` - 구현
- `Source/Editor/Panel/ScenePanel.h` - 마우스 상태 변수 추가
- `Source/Editor/Panel/ScenePanel.cpp` - handleCameraInput() 구현
- `Source/Editor/Core/EditorWindow.h` - RotateSkyCamera() 추가
- `Source/Editor/Core/EditorWindow.cpp` - 구현

### 6. 테스트용 구체 추가
- 하드코딩된 구체로 그림자 테스트용
- Ray-sphere intersection 사용

**파일**: `Shader/Slang/Atmosphere/Sky.frag.slang`
```hlsl
float3 sphereCenter = float3(0.0, 100.0, 100.0);  // 카메라와 같은 높이, 100m 앞
float sphereRadius = 20.0;
```

### 7. GLM-HLSL 행렬 레이아웃 불일치 수정 (최신 작업)
- **문제**: GLM은 column-major, HLSL은 row-major 사용
- **해결**: `inverseViewProjection` 행렬을 transpose하여 전달

**파일**: `Source/Engine/Renderer/Atmosphere/Private/AtmosphereSkyPass.cpp`
```cpp
void AtmosphereSkyPass::updateUniforms()
{
    // GLM uses column-major matrices, but HLSL/Slang expects row-major by default
    // So we need to transpose the matrix
    glm::mat4 viewProj = _projectionMatrix * _viewMatrix;
    glm::mat4 invViewProj = glm::inverse(viewProj);
    _uniforms.inverseViewProjection = glm::transpose(invViewProj);
    // ...
}
```

## Current Issues (미해결)

### 1. 대각선 지평선 문제
- 지평선이 대각선으로 기울어져 있음
- 원인 추정: GLM(column-major)과 HLSL(row-major) 행렬 레이아웃 불일치
- 시도했던 해결책: `glm::transpose(invViewProj)` - 효과 미확인 상태에서 코드 되돌림

### 2. 해결 방향 후보
1. **행렬 transpose**: GLM 행렬을 HLSL에 전달하기 전 transpose
2. **셰이더 측 수정**: `mul(matrix, vector)` 대신 `mul(vector, matrix)` 사용
3. **row_major 지정**: HLSL에서 `row_major float4x4` 명시적 지정

## Next Steps

1. **대각선 지평선 문제 해결**: 행렬 레이아웃 문제 수정 (위 해결 방향 후보 참조)
2. **카메라 회전 기능 재구현**: 지평선 문제 해결 후
3. **테스트용 구체 추가**: 그림자 테스트용

## Key Files

### C++ 소스
- `Source/Engine/Renderer/Atmosphere/AtmosphereSkyPass.h`
- `Source/Engine/Renderer/Atmosphere/Private/AtmosphereSkyPass.cpp`
- `Source/Engine/Renderer/Atmosphere/AtmosphereRenderer.h`
- `Source/Engine/Renderer/Atmosphere/Private/AtmosphereRenderer.cpp`
- `Source/Engine/Renderer/Atmosphere/Private/AtmospherePrecompute.cpp`
- `Source/Editor/Panel/ScenePanel.cpp`
- `Source/Editor/Core/EditorWindow.cpp`
- `Source/RHI/Vulkan/Private/VulkanCommandHandle.cpp`
- `Source/RHI/Vulkan/Private/VulkanContext.cpp`

### 셰이더
- `Shader/Slang/Atmosphere/Sky.vert.slang` - 버텍스 셰이더 (viewRay 계산)
- `Shader/Slang/Atmosphere/Sky.frag.slang` - 프래그먼트 셰이더 (대기 렌더링)
- `Shader/Slang/Atmosphere/AtmosphereAPI.hlsli` - 대기 산란 함수들

### 좌표계 정리
- **월드 좌표계**: 카메라가 (0, 100, 0)에 위치 (지면에서 100m)
- **지구 중심**: (0, -6360000, 0) (월드 좌표)
- **대기 상단**: 지구 반경 6360km + 대기 높이 60km = 6420km
- **행성 중심 좌표**: `camera = cameraPosition - earthCenter = (0, 6360100, 0)`

## Build Commands
```bash
cmake --build C:/Dev/HSMR/Build --config Debug
cd C:/Dev/HSMR/Build/Debug && ./HSMR.exe
```

## 참고 문헌
- Bruneton & Neyret, "Precomputed Atmospheric Scattering" (2008)
- https://github.com/ebruneton/precomputed_atmospheric_scattering
