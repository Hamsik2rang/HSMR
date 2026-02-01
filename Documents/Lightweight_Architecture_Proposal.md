# HSMR 경량화 아키텍처 제안

> **목표**: 렌더 패스 하나를 실행하기 위한 인프라 코드를 최소화
> **작성일**: 2026-01-25

---

## 1. 현재 구조의 문제점

### 렌더 패스 실행을 위한 의존성 체인

```
main.cpp
  → EditorApplication (Editor 모듈)
    → Application (Engine 모듈)
      → Window (Engine 모듈)
        → NativeWindow (Platform 모듈)
        → Swapchain (RHI 모듈)
      → RenderPath (Engine 모듈)
        → RHIContext (RHI 모듈)
        → RenderPass (Engine 모듈)
        → RHIHandleCache
      → GUIContext (Editor 모듈)
        → ImGui 통합
```

**문제**: 삼각형 하나 그리려면 6개 모듈, 10+ 클래스를 거쳐야 함

### 현재 LOC 분포

| 모듈 | LOC | 실제 렌더링 기여도 |
|------|-----|-------------------|
| RHI | 8,691 | 필수 (API 추상화) |
| Engine | 4,826 | 대부분 불필요한 추상화 |
| Editor | 2,219 | 프로토타이핑에 불필요 |
| Platform | 1,863 | 단순화 가능 |
| Core | 1,559 | 일부만 필요 |
| **합계** | **19,200** | |

---

## 2. 제안: 2-레이어 아키텍처

```
┌─────────────────────────────────────────────────────────┐
│                    Samples/                              │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐       │
│  │ Triangle    │ │ SSAO        │ │ Atmosphere  │ ...   │
│  │ main.cpp    │ │ main.cpp    │ │ main.cpp    │       │
│  └──────┬──────┘ └──────┬──────┘ └──────┬──────┘       │
│         │               │               │               │
│         └───────────────┼───────────────┘               │
│                         ▼                               │
│  ┌──────────────────────────────────────────────────┐  │
│  │                    Framework/                     │  │
│  │  ┌────────┐ ┌────────┐ ┌────────┐ ┌───────────┐ │  │
│  │  │ App    │ │ RHI    │ │ Shader │ │ Utility   │ │  │
│  │  │ (~200) │ │ (~4K)  │ │ (~500) │ │ (~500)    │ │  │
│  │  └────────┘ └────────┘ └────────┘ └───────────┘ │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘

예상 총 규모: ~5,200 LOC (현재의 27%)
```

---

## 3. Framework 레이어 상세 설계

### 3.1 App 모듈 (~200 LOC)

```cpp
// Framework/App.h
#pragma once
#include "RHI/RHIContext.h"
#include <functional>

namespace hs {

struct AppConfig {
    const char* title = "HSMR Sample";
    uint32_t width = 1280;
    uint32_t height = 720;
    bool vsync = true;
    ERHIPlatform api = ERHIPlatform::METAL;  // 또는 VULKAN
};

class App {
public:
    App(const AppConfig& config);
    ~App();

    // 메인 루프 - 콜백 기반
    void Run(std::function<void(float dt)> onUpdate,
             std::function<void(RHICommandBuffer*)> onRender);

    // Getter
    RHIContext* GetRHI() { return _rhi; }
    Swapchain* GetSwapchain() { return _swapchain; }
    float GetAspectRatio() { return (float)_width / _height; }
    uint32_t GetWidth() { return _width; }
    uint32_t GetHeight() { return _height; }

    // 입력
    bool IsKeyPressed(int key);
    bool IsKeyDown(int key);
    bool IsMouseButtonPressed(int button);
    glm::vec2 GetMousePosition();
    glm::vec2 GetMouseDelta();

    // 윈도우 제어
    void Close() { _running = false; }
    bool ShouldClose() { return !_running; }

private:
    void PollEvents();
    void CreateNativeWindow();
    void DestroyNativeWindow();

    void* _nativeWindow = nullptr;  // NSWindow* 또는 HWND
    RHIContext* _rhi = nullptr;
    Swapchain* _swapchain = nullptr;

    uint32_t _width, _height;
    bool _running = true;
};

} // namespace hs
```

**설계 원칙:**
- Application/Window/EditorApplication 계층을 단일 클래스로 통합
- 콜백 기반으로 상속 없이 사용
- 플랫폼 코드는 `.mm`/`.cpp` 구현 파일에 숨김

### 3.2 RHI 모듈 (~4,000 LOC)

기존 RHI 인터페이스를 유지하되 불필요한 부분 제거:

**유지:**
```cpp
// RHIContext 인터페이스 (기존과 동일)
class RHIContext {
    // 리소스 생성/파괴
    RHIBuffer* CreateBuffer(...);
    RHITexture* CreateTexture(...);
    RHIShader* CreateShader(...);
    RHIGraphicsPipeline* CreateGraphicsPipeline(...);
    RHIComputePipeline* CreateComputePipeline(...);
    RHIRenderPass* CreateRenderPass(...);
    RHIFramebuffer* CreateFramebuffer(...);
    // ...

    // 커맨드 버퍼
    RHICommandBuffer* CreateCommandBuffer(...);
    void Submit(...);
    void Present(...);

    // 팩토리
    static RHIContext* Create(ERHIPlatform platform);
};

// Metal/Vulkan 구현
class MetalContext : public RHIContext { ... };
class VulkanContext : public RHIContext { ... };
```

**제거:**
- `RenderPath` 클래스
- `RenderPass` 추상 클래스 (Sample에서 직접 구현)
- `RHIHandleCache` (필요한 Sample에서 자체 구현)
- `RenderTarget` 복잡한 추상화

### 3.3 Shader 모듈 (~500 LOC)

```cpp
// Framework/Shader.h
#pragma once
#include "RHI/RHIContext.h"
#include <functional>

namespace hs {

class ShaderCompiler {
public:
    // 파일에서 컴파일
    static RHIShader* CompileFromFile(
        RHIContext* rhi,
        const char* slangPath,
        EShaderStage stage,
        const char* entryPoint = "main"
    );

    // 문자열에서 컴파일 (인라인 셰이더용)
    static RHIShader* CompileFromSource(
        RHIContext* rhi,
        const char* source,
        EShaderStage stage,
        const char* entryPoint = "main"
    );

    // 핫 리로드 지원
    static void EnableHotReload(const char* watchDirectory);
    static void SetReloadCallback(std::function<void(const char* path)> callback);
    static void PollChanges();  // 매 프레임 호출
};

// 간편 매크로
#define HS_LOAD_VS(rhi, path) hs::ShaderCompiler::CompileFromFile(rhi, path, EShaderStage::VERTEX)
#define HS_LOAD_FS(rhi, path) hs::ShaderCompiler::CompileFromFile(rhi, path, EShaderStage::FRAGMENT)
#define HS_LOAD_CS(rhi, path) hs::ShaderCompiler::CompileFromFile(rhi, path, EShaderStage::COMPUTE)

} // namespace hs
```

### 3.4 Utility 모듈 (~500 LOC)

```cpp
// Framework/Utility.h
#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace hs {

//=============================================================================
// 수학 유틸리티
//=============================================================================
using namespace glm;

// 카메라 헬퍼
mat4 CreatePerspective(float fovY, float aspect, float near, float far);
mat4 CreateOrthographic(float left, float right, float bottom, float top, float near, float far);
mat4 CreateLookAt(vec3 eye, vec3 target, vec3 up);

//=============================================================================
// 지오메트리 생성
//=============================================================================
struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};

std::vector<Vertex> CreateFullscreenQuad();  // NDC 전체 화면
std::vector<Vertex> CreateQuad(float size = 1.0f);
std::vector<Vertex> CreateCube(float size = 1.0f);
std::vector<Vertex> CreateSphere(float radius = 1.0f, int segments = 32);
std::vector<Vertex> CreatePlane(float width, float depth, int subdivisions = 1);

//=============================================================================
// 이미지 로딩
//=============================================================================
struct ImageData {
    uint8_t* pixels = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;

    bool IsValid() const { return pixels != nullptr; }
    void Free();
};

ImageData LoadImage(const char* path);
ImageData LoadImageFromMemory(const uint8_t* data, size_t size);

// RHI 텍스처로 직접 로드
RHITexture* LoadTexture(RHIContext* rhi, const char* path, bool generateMips = true);

//=============================================================================
// ImGui 통합 (선택적)
//=============================================================================
#ifdef HS_USE_IMGUI

void ImGuiInit(RHIContext* rhi, Swapchain* swapchain);
void ImGuiNewFrame();
void ImGuiRender(RHICommandBuffer* cmd);
void ImGuiShutdown();

// 디버그 패널 헬퍼
void ImGuiShowFPS();
void ImGuiShowGPUStats(RHIContext* rhi);

#endif

//=============================================================================
// 타이머
//=============================================================================
class Timer {
public:
    void Start();
    void Stop();
    float GetElapsedMs() const;
    float GetElapsedSeconds() const;

private:
    // 플랫폼별 구현
};

//=============================================================================
// 파일 시스템
//=============================================================================
std::string ReadTextFile(const char* path);
std::vector<uint8_t> ReadBinaryFile(const char* path);
bool FileExists(const char* path);
std::string GetExecutablePath();
std::string GetResourcePath(const char* relativePath);

} // namespace hs
```

---

## 4. Sample 작성 예시

### 4.1 기본 삼각형 (~60 LOC)

```cpp
// Samples/Triangle/main.cpp
#include "Framework/App.h"
#include "Framework/Shader.h"

int main() {
    hs::App app({
        .title = "Triangle",
        .width = 800,
        .height = 600,
        .api = hs::ERHIPlatform::METAL
    });

    auto* rhi = app.GetRHI();

    // 버텍스 데이터
    float vertices[] = {
        // position          // color
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f
    };
    auto* vbo = rhi->CreateBuffer("vbo", vertices, sizeof(vertices),
                                   EBufferUsage::VERTEX, EBufferMemoryOption::STATIC);

    // 셰이더
    auto* vs = HS_LOAD_VS(rhi, "Shaders/triangle.vert.slang");
    auto* fs = HS_LOAD_FS(rhi, "Shaders/triangle.frag.slang");

    // 파이프라인
    GraphicsPipelineInfo pipelineInfo = {};
    // ... 파이프라인 설정 ...
    auto* pipeline = rhi->CreateGraphicsPipeline("main", pipelineInfo);

    // 메인 루프
    app.Run(
        // Update
        [&](float dt) {
            if (app.IsKeyPressed(KEY_ESCAPE)) {
                app.Close();
            }
        },
        // Render
        [&](RHICommandBuffer* cmd) {
            cmd->BeginRenderPass(/* swapchain render pass */);
            cmd->SetViewport(0, 0, app.GetWidth(), app.GetHeight());
            cmd->BindPipeline(pipeline);
            cmd->BindVertexBuffer(vbo, 0);
            cmd->Draw(3, 0);
            cmd->EndRenderPass();
        }
    );

    // 정리
    rhi->DestroyGraphicsPipeline(pipeline);
    rhi->DestroyShader(vs);
    rhi->DestroyShader(fs);
    rhi->DestroyBuffer(vbo);

    return 0;
}
```

### 4.2 SSAO 구현 (~200 LOC)

```cpp
// Samples/SSAO/main.cpp
#include "Framework/App.h"
#include "Framework/Shader.h"
#include "Framework/Utility.h"

struct SSAOParams {
    float radius = 0.5f;
    float bias = 0.025f;
    int kernelSize = 64;
};

int main() {
    hs::App app({ .title = "SSAO Demo", .width = 1280, .height = 720 });
    auto* rhi = app.GetRHI();

    // G-Buffer 텍스처
    auto* gPosition = rhi->CreateTexture("gPosition", nullptr,
        app.GetWidth(), app.GetHeight(), EPixelFormat::RGBA16F, ...);
    auto* gNormal = rhi->CreateTexture("gNormal", nullptr,
        app.GetWidth(), app.GetHeight(), EPixelFormat::RGBA16F, ...);
    auto* gAlbedo = rhi->CreateTexture("gAlbedo", nullptr,
        app.GetWidth(), app.GetHeight(), EPixelFormat::RGBA8, ...);

    // SSAO 텍스처
    auto* ssaoTexture = rhi->CreateTexture("ssao", nullptr,
        app.GetWidth(), app.GetHeight(), EPixelFormat::R8, ...);

    // 노이즈 텍스처 & 커널 생성
    auto* noiseTexture = CreateSSAONoise(rhi);
    auto* kernelBuffer = CreateSSAOKernel(rhi, 64);

    // 셰이더 & 파이프라인
    auto* geometryPass = CreateGeometryPassPipeline(rhi);
    auto* ssaoPass = CreateSSAOPassPipeline(rhi);
    auto* blurPass = CreateBlurPassPipeline(rhi);
    auto* lightingPass = CreateLightingPassPipeline(rhi);

    // 씬 데이터
    auto mesh = hs::LoadMesh("Models/sponza.obj");
    auto* vertexBuffer = rhi->CreateBuffer("mesh", mesh.data(), ...);

    // 카메라
    hs::Camera camera;
    camera.SetPerspective(45.0f, app.GetAspectRatio(), 0.1f, 1000.0f);

    SSAOParams params;

    app.Run(
        [&](float dt) {
            camera.Update(app, dt);

            // ImGui로 파라미터 조정
            hs::ImGuiNewFrame();
            ImGui::SliderFloat("Radius", &params.radius, 0.1f, 2.0f);
            ImGui::SliderFloat("Bias", &params.bias, 0.0f, 0.1f);
        },
        [&](RHICommandBuffer* cmd) {
            // 1. Geometry Pass → G-Buffer
            cmd->BeginRenderPass(gBufferRenderPass);
            cmd->BindPipeline(geometryPass);
            cmd->Draw(...);
            cmd->EndRenderPass();

            // 2. SSAO Pass
            cmd->BeginRenderPass(ssaoRenderPass);
            cmd->BindPipeline(ssaoPass);
            cmd->BindTexture(0, gPosition);
            cmd->BindTexture(1, gNormal);
            cmd->BindTexture(2, noiseTexture);
            cmd->DrawFullscreenQuad();
            cmd->EndRenderPass();

            // 3. Blur Pass
            // ...

            // 4. Lighting Pass (final)
            cmd->BeginRenderPass(swapchainRenderPass);
            cmd->BindPipeline(lightingPass);
            cmd->BindTexture(0, gAlbedo);
            cmd->BindTexture(1, ssaoTexture);
            cmd->DrawFullscreenQuad();

            hs::ImGuiRender(cmd);
            cmd->EndRenderPass();
        }
    );

    // 정리...
    return 0;
}
```

---

## 5. 제거되는 구성요소

| 현재 구성요소 | 상태 | 이유 |
|--------------|------|------|
| **Editor 모듈 전체** | 제거 | ImGui는 Utility로 이동 |
| **Application 클래스** | 제거 → `App` | 200 LOC로 단순화 |
| **Window 클래스** | 제거 → `App` 내부 | 추상화 불필요 |
| **EditorApplication** | 제거 | 불필요 |
| **RenderPath** | 제거 | Sample에서 직접 구현 |
| **RenderPass 추상화** | 제거 | Sample에서 직접 커맨드 기록 |
| **RHIHandleCache** | 제거 | 필요한 Sample만 자체 구현 |
| **ObjectManager** | 제거 | `Utility` 함수로 대체 |
| **Proxy 시스템 전체** | 제거 | RHI 리소스 직접 사용 |
| **EngineContext** | 제거 | 불필요 |
| **Platform 모듈** | 축소 | `App` 내부로 통합 |
| **Core 모듈 일부** | 축소 | 필요한 것만 `Utility`로 |

---

## 6. LOC 비교

| 구성요소 | 현재 | 제안 | 감소율 |
|---------|------|------|--------|
| App/Window/Platform | 4,082 | 200 | **95%** |
| RHI | 8,691 | 4,000 | 54% |
| Engine (Renderer 등) | 4,826 | 0 | **100%** |
| Editor | 2,219 | 0 | **100%** |
| Shader | (분산) | 500 | - |
| Utility | (분산) | 500 | - |
| **합계** | **19,200** | **~5,200** | **73%** |

---

## 7. 디렉토리 구조

```
HSMR/
├── Framework/
│   ├── App.h
│   ├── App.cpp              # 공통 로직
│   ├── App_Metal.mm         # macOS 구현
│   ├── App_Win32.cpp        # Windows 구현
│   │
│   ├── RHI/
│   │   ├── RHIContext.h
│   │   ├── RHIDefinition.h
│   │   ├── RHIHandle.h
│   │   ├── Metal/
│   │   │   ├── MetalContext.h
│   │   │   └── MetalContext.mm
│   │   └── Vulkan/
│   │       ├── VulkanContext.h
│   │       └── VulkanContext.cpp
│   │
│   ├── Shader.h
│   ├── Shader.cpp
│   │
│   ├── Utility.h
│   └── Utility.cpp
│
├── Samples/
│   ├── 01_Triangle/
│   │   ├── main.cpp
│   │   └── CMakeLists.txt
│   │
│   ├── 02_Texture/
│   │   ├── main.cpp
│   │   └── CMakeLists.txt
│   │
│   ├── 03_SSAO/
│   │   ├── main.cpp
│   │   ├── SSAOPass.h        # 이 Sample 전용 헬퍼
│   │   └── CMakeLists.txt
│   │
│   ├── 04_Atmosphere/
│   │   ├── main.cpp
│   │   ├── AtmosphereCompute.h
│   │   └── CMakeLists.txt
│   │
│   └── CMakeLists.txt        # 전체 Sample 빌드
│
├── Shaders/
│   ├── Common/
│   │   ├── Math.slang
│   │   └── Lighting.slang
│   ├── Triangle/
│   │   ├── triangle.vert.slang
│   │   └── triangle.frag.slang
│   ├── SSAO/
│   │   ├── geometry.vert.slang
│   │   ├── ssao.frag.slang
│   │   └── blur.frag.slang
│   └── ...
│
├── Resources/
│   ├── Models/
│   ├── Textures/
│   └── ...
│
├── Source/                   # 기존 코드 (참조용 또는 제거)
│   └── ...
│
└── CMakeLists.txt
```

---

## 8. 마이그레이션 전략

### Phase 1: Framework 기반 구축

1. `Framework/` 폴더 생성
2. `App` 클래스 구현 (Metal 먼저)
3. 기존 RHI에서 필요한 코드만 복사
4. `Utility` 기본 함수 구현

### Phase 2: 첫 번째 Sample

1. `Samples/01_Triangle/` 작성
2. 전체 파이프라인 검증
3. 셰이더 컴파일 워크플로우 확립

### Phase 3: Vulkan 지원 추가

1. `App_Win32.cpp` 구현
2. `VulkanContext` 복사/정리
3. Triangle Sample이 Vulkan에서도 동작 확인

### Phase 4: 기존 코드 정리

1. 새 구조 안정화 확인
2. `Source/` 폴더를 `Legacy/`로 이동 또는 삭제
3. 문서 업데이트

---

## 9. 이 구조의 장점

### 즉시 렌더링 코드 작성 가능
- Sample의 `main.cpp`에서 바로 그래픽스 코드 시작
- 중간 추상화 레이어 없음
- "Hello Triangle"이 60 LOC

### API 학습에 집중
- RHI 레이어가 Vulkan/Metal 차이를 명확히 보여줌
- 추상화가 얇아서 네이티브 API 동작 이해 용이
- 각 Sample에서 API 호출 흐름이 직접 보임

### 기법별 독립성
- 각 Sample이 완전히 독립적
- 한 Sample 수정이 다른 Sample에 영향 없음
- 실패한 실험을 폴더째 삭제 가능

### 빠른 빌드
- 5,200 LOC = 빌드 시간 대폭 단축
- Sample별 개별 빌드 가능
- 수정 → 확인 사이클 단축

### 유연한 확장
- 복잡한 기법은 Sample 내에서 자체 클래스 구현
- 단순한 기법은 `main.cpp` 인라인으로 충분
- 공통 패턴이 발견되면 `Utility`로 승격

---

## 10. 고려사항

### ImGui 통합
- `#define HS_USE_IMGUI`로 선택적 활성화
- 파라미터 튜닝이 필요한 Sample만 사용
- 성능 프로파일링 표시에도 활용

### 셰이더 핫 리로드
- `ShaderCompiler::EnableHotReload()` 호출로 활성화
- 파일 변경 감지 → 콜백 호출
- Sample에서 파이프라인 재생성 처리

### 멀티 플랫폼 빌드
```cmake
# CMakeLists.txt
if(APPLE)
    set(PLATFORM_SOURCES App_Metal.mm)
    set(RHI_BACKEND Metal)
else()
    set(PLATFORM_SOURCES App_Win32.cpp)
    set(RHI_BACKEND Vulkan)
endif()
```

---

*이 문서는 HSMR 프로젝트의 경량화 아키텍처 제안입니다. 기존 코드를 완전히 대체하거나 병행 운영할 수 있습니다.*
