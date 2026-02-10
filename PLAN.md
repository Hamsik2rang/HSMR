# Tracy Profiler 통합 및 Profiler 패널 고도화 계획

## 현재 상태 분석

### 기존 프로파일링 시스템
| 파일 | 상태 | 설명 |
|:-----|:-----|:-----|
| `Core/Profiler/Profiler.h` | **전체 주석 처리** | ImPlot 기반 자체 구현 시도 흔적 |
| `Core/Profiler/GPUQuery.h/.cpp` | 스켈레톤 | RHI 연동 없음, placeholder 상태 |
| `Editor/Panel/ProfilerPanel.h/.cpp` | 최소 기능 | FPS + Camera 정보만 표시 |

### 문제점
1. GPU 프로파일링 미구현 (GPUQuery가 placeholder)
2. CPU 프로파일링 비활성화 (전체 주석)
3. ProfilerPanel이 단순 오버레이 수준
4. 외부 도구 연동 없음

---

## Tracy 도입 근거

### Tracy란?
- **실시간 프레임 프로파일러** (게임/실시간 그래픽스 특화)
- CPU/GPU 동시 프로파일링 지원
- Vulkan 네이티브 지원 (`VK_EXT_calibrated_timestamps`)
- 메모리 할당 추적
- Lock contention 분석
- 원격 프로파일링 (별도 GUI 앱)

### 상용 엔진과의 비교
| 엔진 | 프로파일러 |
|:-----|:----------|
| Unreal | Unreal Insights (자체) |
| Unity | Unity Profiler + external tools |
| Godot | Tracy 내장 |
| HSMR (목표) | **Tracy 통합** |

### 학습 가치
- 프로파일러 **사용법**은 학습 가치 있음 ✅
- 프로파일러 **구현**은 핵심 목표 아님 → Tracy 채택 적절
- GPU timestamp 쿼리 연동은 RHI 이해에 도움 (보호 영역 경계)

---

## 아키텍처 설계

```
┌─────────────────────────────────────────────────────────────┐
│                     Tracy Server (GUI)                       │
│                   (별도 프로세스 실행)                         │
└────────────────────────────┬────────────────────────────────┘
                             │ TCP (localhost:8086)
                             ▼
┌─────────────────────────────────────────────────────────────┐
│                        HSMR Engine                           │
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │  HS_ZONE(name)  │  │ HS_GPU_ZONE(cmd)│  │ HS_ALLOC(ptr)│ │
│  │  CPU 구간 측정   │  │  GPU 구간 측정   │  │ 메모리 추적   │ │
│  └────────┬────────┘  └────────┬────────┘  └──────┬───────┘ │
│           │                    │                   │         │
│           └──────────┬─────────┴───────────────────┘         │
│                      ▼                                       │
│           ┌──────────────────────┐                           │
│           │    TracyClient.cpp   │                           │
│           │   (Tracy 네트워크)    │                           │
│           └──────────────────────┘                           │
│                                                              │
│  ┌────────────────────────────────────────────────────────┐  │
│  │              ProfilerPanel (고도화)                     │  │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌───────────┐  │  │
│  │  │Frame Time│ │CPU Zones │ │GPU Zones │ │Memory Info│  │  │
│  │  │  Graph   │ │  Table   │ │  Table   │ │   Stats   │  │  │
│  │  └──────────┘ └──────────┘ └──────────┘ └───────────┘  │  │
│  └────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

---

## 구현 범위 분류

### AI 구현 가능 (보일러플레이트)
| 작업 | 이유 |
|:-----|:-----|
| Tracy 라이브러리 다운로드/통합 | 외부 라이브러리 설정 |
| 매크로 래퍼 정의 | 단순 래핑 |
| ProfilerPanel UI 고도화 | ImGui 위젯 |
| CMake 설정 | 빌드 스크립트 |

### 사용자 직접 구현 권장 (보호 영역 경계)
| 작업 | 이유 |
|:-----|:-----|
| GPU Timestamp Query 연동 | Vulkan 동기화/쿼리 이해 필요 |
| Calibrated Timestamps 활용 | CPU-GPU 시간 동기화 학습 |
| VulkanCommandHandle 확장 | RHI 아키텍처 이해 |

---

## 1단계: Tracy 라이브러리 통합

### 1.1 다운로드
```
Dependency/
├── include/
│   └── tracy/
│       ├── Tracy.hpp              (메인 헤더)
│       ├── TracyVulkan.hpp        (Vulkan GPU 프로파일링)
│       └── ...
└── lib/
    └── tracy/
        └── TracyClient.cpp        (단일 소스 - 정적 빌드)
```

**버전**: v0.11.1 (최신 안정)
**URL**: https://github.com/wolfpld/tracy/releases

### 1.2 CMake 설정

```cmake
# Dependency/CMakeLists.txt 또는 root CMakeLists.txt

option(HS_ENABLE_TRACY "Enable Tracy profiler" ON)

if(HS_ENABLE_TRACY)
    add_library(TracyClient STATIC
        ${HS_DEPS_DIR}/lib/tracy/TracyClient.cpp
    )
    target_include_directories(TracyClient PUBLIC
        ${HS_DEPS_INCLUDE_DIR}/tracy
    )
    target_compile_definitions(TracyClient PUBLIC
        TRACY_ENABLE
        TRACY_ON_DEMAND          # 서버 연결 시에만 프로파일링
        TRACY_NO_EXIT            # 종료 시 데이터 손실 방지
        TRACY_CALLSTACK=16       # 콜스택 깊이
    )

    # Vulkan GPU 프로파일링 활성화
    if(VULKAN_FOUND)
        target_compile_definitions(TracyClient PUBLIC
            TRACY_VK_USE_SYMBOL_TABLE
        )
    endif()
endif()
```

### 1.3 Engine 링크

```cmake
# Source/Engine/CMakeLists.txt
if(HS_ENABLE_TRACY)
    target_link_libraries(Engine PUBLIC TracyClient)
endif()
```

---

## 2단계: 프로파일링 매크로 래퍼

### 2.1 헤더 파일

**파일**: `Source/Core/Profiler/ProfilerMacros.h`

```cpp
#pragma once

#include "Precompile.h"

#if defined(TRACY_ENABLE)
    #include <tracy/Tracy.hpp>
    #include <tracy/TracyVulkan.hpp>

    // CPU 프로파일링
    #define HS_PROFILE_FRAME_MARK       FrameMark
    #define HS_PROFILE_ZONE(name)       ZoneScoped
    #define HS_PROFILE_ZONE_N(name)     ZoneScopedN(name)
    #define HS_PROFILE_ZONE_C(name, color) ZoneScopedNC(name, color)
    #define HS_PROFILE_FUNCTION()       ZoneScoped

    // GPU 프로파일링 (Vulkan)
    #define HS_PROFILE_GPU_CONTEXT(ctx, device, queue, cmdBuffer, ...) \
        TracyVkContext(ctx, device, queue, cmdBuffer, ##__VA_ARGS__)
    #define HS_PROFILE_GPU_ZONE(ctx, cmdBuffer, name) \
        TracyVkZone(ctx, cmdBuffer, name)
    #define HS_PROFILE_GPU_COLLECT(ctx, cmdBuffer) \
        TracyVkCollect(ctx, cmdBuffer)

    // 메모리 프로파일링
    #define HS_PROFILE_ALLOC(ptr, size)   TracyAlloc(ptr, size)
    #define HS_PROFILE_FREE(ptr)          TracyFree(ptr)

    // 값 플로팅
    #define HS_PROFILE_PLOT(name, value)  TracyPlot(name, value)

    // 메시지/로그
    #define HS_PROFILE_MESSAGE(text)      TracyMessage(text, strlen(text))

#else
    // Tracy 비활성화 시 no-op
    #define HS_PROFILE_FRAME_MARK
    #define HS_PROFILE_ZONE(name)
    #define HS_PROFILE_ZONE_N(name)
    #define HS_PROFILE_ZONE_C(name, color)
    #define HS_PROFILE_FUNCTION()

    #define HS_PROFILE_GPU_CONTEXT(...)
    #define HS_PROFILE_GPU_ZONE(...)
    #define HS_PROFILE_GPU_COLLECT(...)

    #define HS_PROFILE_ALLOC(ptr, size)
    #define HS_PROFILE_FREE(ptr)

    #define HS_PROFILE_PLOT(name, value)
    #define HS_PROFILE_MESSAGE(text)
#endif
```

### 2.2 사용 예시

```cpp
// Application 메인 루프
void PrototypeApplication::Tick()
{
    HS_PROFILE_FRAME_MARK;  // 프레임 경계 표시

    {
        HS_PROFILE_ZONE_N("Update");
        Update();
    }

    {
        HS_PROFILE_ZONE_N("Render");
        Render();
    }
}

// 렌더링 함수 내부
void Renderer::DrawScene(RHICommandBuffer* cmd)
{
    HS_PROFILE_FUNCTION();

    // GPU 프로파일링 (사용자 구현 필요)
    // HS_PROFILE_GPU_ZONE(_tracyVkCtx, cmd->handle, "DrawScene");

    for (auto& object : objects)
    {
        HS_PROFILE_ZONE_N("DrawObject");
        DrawObject(object);
    }
}
```

---

## 3단계: ProfilerPanel 고도화

### 3.1 새로운 구조

**파일**: `Source/Editor/Panel/ProfilerPanel.h`

```cpp
#pragma once
#include "Precompile.h"
#include "Editor/Panel/Panel.h"

HS_NS_EDITOR_BEGIN

class HS_EDITOR_API ProfilerPanel : public Panel
{
public:
    ProfilerPanel(Window* window);
    ~ProfilerPanel() override;

    bool Setup() override;
    void Cleanup() override;
    void Draw() override;

private:
    // UI 섹션 그리기
    void DrawFrameTimeSection();
    void DrawCPUSection();
    void DrawGPUSection();
    void DrawMemorySection();
    void DrawSettingsSection();

    // 데이터
    struct FrameData
    {
        float frameTime = 0.0f;
        float cpuTime = 0.0f;
        float gpuTime = 0.0f;
        float fps = 0.0f;
    };

    static constexpr int HISTORY_SIZE = 256;
    std::array<FrameData, HISTORY_SIZE> _frameHistory;
    int _frameIndex = 0;

    // 설정
    bool _showCPU = true;
    bool _showGPU = true;
    bool _showMemory = true;
    bool _pauseProfiling = false;
    float _targetFrameTime = 16.67f;  // 60 FPS
};

HS_NS_EDITOR_END
```

### 3.2 UI 구현 개요

```cpp
void ProfilerPanel::Draw()
{
    ImGui::Begin("Profiler", nullptr, ImGuiWindowFlags_MenuBar);

    // 메뉴바
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("View"))
        {
            ImGui::Checkbox("CPU Timings", &_showCPU);
            ImGui::Checkbox("GPU Timings", &_showGPU);
            ImGui::Checkbox("Memory", &_showMemory);
            ImGui::EndMenu();
        }

        // Tracy 연결 상태
        if (ImGui::MenuItem(_pauseProfiling ? "Resume" : "Pause"))
        {
            _pauseProfiling = !_pauseProfiling;
        }

        ImGui::EndMenuBar();
    }

    // 프레임 타임 그래프 (항상 표시)
    DrawFrameTimeSection();

    ImGui::Separator();

    // 탭으로 섹션 구분
    if (ImGui::BeginTabBar("ProfilerTabs"))
    {
        if (_showCPU && ImGui::BeginTabItem("CPU"))
        {
            DrawCPUSection();
            ImGui::EndTabItem();
        }

        if (_showGPU && ImGui::BeginTabItem("GPU"))
        {
            DrawGPUSection();
            ImGui::EndTabItem();
        }

        if (_showMemory && ImGui::BeginTabItem("Memory"))
        {
            DrawMemorySection();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Settings"))
        {
            DrawSettingsSection();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
```

---

## 4단계: GPU 프로파일링 (사용자 직접 구현)

### 4.1 VulkanDevice 확장 (직접 구현 권장)

```cpp
// Source/RHI/Vulkan/VulkanDevice.h 에 추가
class VulkanDevice
{
public:
    // ... 기존 코드 ...

    // [사용자 직접 구현] GPU 프로파일링 지원
    float timestampPeriod = 0.0f;  // 나노초/틱
    bool supportsTimestamps = false;

    // Tracy Vulkan Context (optional)
    #if defined(TRACY_ENABLE)
    TracyVkCtx tracyVkContext = nullptr;
    #endif
};
```

### 4.2 Tracy Vulkan Context 초기화 (직접 구현 권장)

```cpp
// VulkanDevice::Create() 또는 별도 함수
void VulkanDevice::InitializeTracyContext(VkCommandBuffer setupCmdBuffer)
{
    #if defined(TRACY_ENABLE)
    // 1. Timestamp period 가져오기
    timestampPeriod = properties.limits.timestampPeriod;
    supportsTimestamps = (timestampPeriod > 0);

    if (!supportsTimestamps)
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
    #endif
}
```

### 4.3 GPU Zone 삽입 위치 (직접 구현 권장)

```cpp
// RenderPass 시작/끝 또는 주요 렌더링 구간에 삽입
void Renderer::ExecuteRenderGraph(...)
{
    auto* cmd = GetCommandBuffer();

    #if defined(TRACY_ENABLE)
    TracyVkZone(g_device->tracyVkContext, cmd->handle, "RenderGraph");
    #endif

    // ... 렌더링 코드 ...

    #if defined(TRACY_ENABLE)
    TracyVkCollect(g_device->tracyVkContext, cmd->handle);
    #endif
}
```

---

## 5단계: 메모리 프로파일링

### 5.1 할당자 연동 (선택사항)

```cpp
// 커스텀 할당자 사용 시
void* CustomAllocator::Allocate(size_t size)
{
    void* ptr = malloc(size);
    HS_PROFILE_ALLOC(ptr, size);
    return ptr;
}

void CustomAllocator::Free(void* ptr)
{
    HS_PROFILE_FREE(ptr);
    free(ptr);
}
```

---

## 구현 순서

| 순서 | 작업 | 담당 | 파일 |
|:----:|:-----|:----:|:-----|
| 1 | Tracy 라이브러리 다운로드 | AI | `Dependency/include/tracy/` |
| 2 | CMake 설정 (TracyClient 빌드) | AI | `CMakeLists.txt` |
| 3 | ProfilerMacros.h 작성 | AI | `Source/Core/Profiler/` |
| 4 | 기존 Profiler.h 정리/제거 | AI | 주석 제거 or 삭제 |
| 5 | Application에 Frame Mark 삽입 | AI | `PrototypeApplication.cpp` |
| 6 | 주요 함수에 Zone 삽입 | AI | 엔진 전반 |
| 7 | ProfilerPanel UI 고도화 | AI | `ProfilerPanel.cpp` |
| 8 | **VulkanDevice Tracy 초기화** | **사용자** | `VulkanDevice.cpp` |
| 9 | **GPU Zone 삽입** | **사용자** | 렌더링 코드 |
| 10 | 빌드 및 테스트 | 공동 | - |

---

## 예상 결과

### Tracy Server에서 확인 가능
1. **프레임 타임라인**: CPU/GPU 병렬 실행 시각화
2. **Zone 히트맵**: 핫스팟 식별
3. **메모리 사용량**: 할당 패턴 분석
4. **콜스택**: 성능 병목 추적
5. **Lock 경합**: 멀티스레드 분석

### ProfilerPanel에서 확인 가능
1. FPS / Frame Time 그래프
2. CPU 주요 구간 소요 시간
3. GPU 렌더패스별 소요 시간 (사용자 구현 후)
4. 메모리 통계

---

## 참고 자료

- [Tracy GitHub](https://github.com/wolfpld/tracy)
- [Tracy Manual (PDF)](https://github.com/wolfpld/tracy/releases/download/v0.11.1/tracy.pdf)
- [Tracy Vulkan Example](https://github.com/wolfpld/tracy/blob/master/examples/OpenGLVulkan/)
- [Godot Tracy Integration](https://github.com/godotengine/godot/blob/master/core/profiler)

---

## 대안 검토

| 도구 | 장점 | 단점 |
|:-----|:-----|:-----|
| **Tracy** ✅ | 게임 특화, GPU 지원, 경량 | 별도 GUI 필요 |
| RenderDoc | GPU 디버깅 특화 | CPU 프로파일링 없음 |
| Intel VTune | 심층 분석 | 무겁고 복잡 |
| Optick | Tracy 유사 | 업데이트 느림 |
| 자체 구현 | 완전한 커스터마이징 | 시간 소모 큼 |

**결론**: Tracy가 이 프로젝트 목적에 가장 적합
