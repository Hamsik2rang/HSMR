# Tracy Profiler Integration

## 작업 완료 내역

Tracy v0.11.1 프로파일러가 HSMR 엔진에 통합되었습니다.

### 추가/수정된 파일

| 파일 | 설명 |
|:-----|:-----|
| `Dependency/include/tracy/` | Tracy 라이브러리 (v0.11.1) |
| `Source/Core/Profiler/ProfilerMacros.h` | Tracy 래퍼 매크로 **[신규]** |
| `Source/Core/Profiler/Profiler.h` | Tracy 통합 헤더로 간소화 |
| `Source/Core/Profiler/Private/Profiler.cpp` | 레거시 코드 제거 |
| `Source/Editor/Panel/ProfilerPanel.h/.cpp` | UI 고도화 |
| `Source/Editor/Core/EditorWindow.cpp` | 프로파일링 존 추가 |
| `Source/Application/PrototypeApplication.cpp` | Frame Mark 및 존 추가 |
| `CMakeLists.txt` | TracyClient 라이브러리 빌드 설정 |
| `Source/Core/CMakeLists.txt` | Tracy 링크 설정 |

---

## 사용법

### 1. Tracy Server 다운로드

GitHub에서 Tracy 릴리즈 다운로드:
https://github.com/wolfpld/tracy/releases

Windows: `Tracy-xxx.7z` 또는 `Tracy.exe` 다운로드

### 2. 애플리케이션 실행

```bash
# Debug 빌드로 실행 (TRACY_ENABLE 자동 정의됨)
./Build/Debug/HSMR.exe
```

### 3. Tracy Server 연결

1. `Tracy.exe` 실행
2. "Connect" 클릭
3. 애플리케이션이 실행 중이면 자동 연결

---

## CPU 프로파일링 매크로

```cpp
#include "Core/Profiler/Profiler.h"

void MyFunction()
{
    // 함수 전체 프로파일링
    HS_PROFILE_FUNCTION();

    {
        // 특정 구간 프로파일링
        HS_PROFILE_ZONE_N("MySection");
        // ... 코드 ...
    }

    {
        // 색상 지정 구간 (Tracy에서 시각적 구분)
        HS_PROFILE_ZONE_NC("Render", HS::Profile::ColorRender);
        // ... 렌더링 코드 ...
    }
}

// 메인 루프에서 프레임 경계 표시
void MainLoop()
{
    while (running)
    {
        HS_PROFILE_FRAME_MARK;  // 프레임 시작 표시
        // ...
    }
}
```

### 사전 정의된 색상

```cpp
namespace HS::Profile
{
    ColorRender   = 0xE91E63;  // Pink - 렌더링
    ColorPhysics  = 0x4CAF50;  // Green - 물리
    ColorAI       = 0xFF9800;  // Orange - AI/로직
    ColorAudio    = 0x9C27B0;  // Purple - 오디오
    ColorNetwork  = 0x00BCD4;  // Cyan - 네트워크
    ColorIO       = 0x795548;  // Brown - 파일 I/O
    ColorMemory   = 0xF44336;  // Red - 메모리
    ColorUI       = 0x2196F3;  // Blue - UI
    ColorScene    = 0xFFEB3B;  // Yellow - 씬 관리
}
```

---

## ProfilerPanel (인앱 오버레이)

에디터 실행 시 좌측 상단에 표시되는 오버레이:

- **FPS 및 프레임 타임** (색상으로 성능 표시)
- **Min/Max 통계**
- **프레임 타임 그래프**
- **카메라 정보**
- **Tracy 연결 상태**

**우클릭 메뉴:**
- 각 섹션 표시/숨김
- 타겟 FPS 설정
- 통계 리셋

---

## 빌드 설정

### Tracy 활성화 (기본값: ON)

```bash
cmake -B build -DHSMR_ENABLE_TRACY=ON
```

### Tracy 비활성화

```bash
cmake -B build -DHSMR_ENABLE_TRACY=OFF
```

Tracy 비활성화 시 모든 `HS_PROFILE_*` 매크로는 no-op으로 컴파일됩니다.

---

## 사용자 추가 작업 (GPU 프로파일링)

GPU 타이밍 측정은 Vulkan 동기화/쿼리 이해가 필요하므로 직접 구현을 권장합니다.

### 1단계: VulkanDevice 확장

```cpp
// Source/RHI/Vulkan/VulkanDevice.h

#if defined(TRACY_ENABLE)
#include <vulkan/vulkan.h>
#include <tracy/TracyVulkan.hpp>  // Vulkan 헤더 다음에 include
#endif

class VulkanDevice
{
public:
    // ... 기존 코드 ...

#if defined(TRACY_ENABLE)
    TracyVkCtx tracyVkContext = nullptr;

    void InitializeTracyContext(VkCommandBuffer setupCmdBuffer);
    void DestroyTracyContext();
#endif
};
```

### 2단계: Tracy Context 초기화

```cpp
// Source/RHI/Vulkan/Private/VulkanDevice.cpp

#if defined(TRACY_ENABLE)
void VulkanDevice::InitializeTracyContext(VkCommandBuffer setupCmdBuffer)
{
    // 1. Timestamp period 확인
    float timestampPeriod = properties.limits.timestampPeriod;
    if (timestampPeriod <= 0)
    {
        HS_LOG(warning, "GPU timestamps not supported");
        return;
    }

    // 2. Tracy Vulkan Context 생성
    tracyVkContext = TracyVkContext(
        physicalDevice,
        logicalDevice,
        graphicsQueue,
        setupCmdBuffer
    );

    HS_LOG(info, "Tracy GPU profiling initialized");
}

void VulkanDevice::DestroyTracyContext()
{
    if (tracyVkContext)
    {
        TracyVkDestroy(tracyVkContext);
        tracyVkContext = nullptr;
    }
}
#endif
```

### 3단계: GPU Zone 사용

```cpp
// 렌더링 코드에서
void Renderer::Render(RHICommandBuffer* cmd)
{
#if defined(TRACY_ENABLE)
    TracyVkZone(g_device->tracyVkContext,
                static_cast<VkCommandBuffer>(cmd->GetNativeHandle()),
                "MainRenderPass");
#endif

    // ... 렌더링 코드 ...
}

// 프레임 끝에서 (Present 전)
void Renderer::EndFrame()
{
#if defined(TRACY_ENABLE)
    TracyVkCollect(g_device->tracyVkContext, currentCmdBuffer);
#endif
}
```

### 학습 포인트

GPU 프로파일링 구현 시 이해해야 할 개념:
1. **Timestamp Query**: `vkCmdWriteTimestamp`로 GPU 타임스탬프 기록
2. **Query Pool**: 타임스탬프 저장 공간 관리
3. **Calibrated Timestamps**: CPU-GPU 시간 동기화 (`VK_EXT_calibrated_timestamps`)
4. **Query 결과 수집**: 프레임 N+2~3에서 결과 읽기 (GPU 지연)

---

## 참고 자료

- [Tracy Manual (PDF)](https://github.com/wolfpld/tracy/releases)
- [Tracy GitHub](https://github.com/wolfpld/tracy)
- [VK_EXT_calibrated_timestamps](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_EXT_calibrated_timestamps.html)

---

## 트러블슈팅

### Tracy Server가 연결되지 않음

1. 방화벽에서 포트 8086 허용
2. `TRACY_ON_DEMAND` 정의로 인해 서버 연결 전에는 데이터 수집 안함
3. 애플리케이션을 먼저 실행한 후 Tracy 연결

### 프레임이 표시되지 않음

`HS_PROFILE_FRAME_MARK`가 메인 루프에서 호출되는지 확인

### GPU 타이밍이 비정상

- `timestampPeriod` 값 확인 (나노초 단위)
- Query Pool 크기가 충분한지 확인
- `TracyVkCollect` 호출 타이밍 확인 (매 프레임 필수)
