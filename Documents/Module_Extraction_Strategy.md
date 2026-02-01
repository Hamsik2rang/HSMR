# HSMR 모듈 분리 및 재활용 전략

> **목표**: HSMR을 엔진 프로젝트로 유지하면서, 핵심 모듈을 DLL/dylib로 분리하여 경량 프로젝트에서 재활용
> **작성일**: 2026-01-25

---

## 1. 전략 개요

### 1.1 현재 상태 vs 목표 상태

```
현재 (모놀리식)                     목표 (모듈화)
─────────────────                  ─────────────────
┌─────────────────┐                ┌─────────────────┐
│     HSMR        │                │   HSMR Engine   │
│  (하나의 덩어리)  │                │  ┌───────────┐  │
│                 │       →        │  │ Editor    │  │
│                 │                │  │ Engine    │  │
│                 │                │  └─────┬─────┘  │
└─────────────────┘                │        │        │
                                   │  ┌─────▼─────┐  │
                                   │  │ Modules/  │◄─┼──── 새 프로젝트에서 링크
                                   │  │ (DLL)     │  │
                                   │  └───────────┘  │
                                   └─────────────────┘
```

### 1.2 핵심 아이디어

| 프로젝트 | 역할 | 사용 모듈 |
|---------|------|----------|
| **HSMR (엔진)** | 풀 기능 렌더링 엔진 | 모든 모듈 |
| **새 프로젝트 (경량)** | 모델 뷰어, 기법 테스트 | Core + RHI + Resource만 링크 |

---

## 2. 모듈 재활용성 분석

### 2.1 재활용성 매트릭스

```
┌──────────────────────┬────────────┬─────────────────┬──────────────────────┐
│ 모듈                 │ 빌드 타입   │ 재활용성         │ 추출 난이도           │
├──────────────────────┼────────────┼─────────────────┼──────────────────────┤
│ Core/*               │ Static     │ ✅✅✅ 100%     │ 없음 (그대로 복사)     │
│ Platform/*           │ Static     │ ✅✅ 95%        │ 낮음 (OS 필터링)      │
│ RHI/*                │ Static     │ ✅✅✅ 100%     │ 없음 (그대로 복사)     │
│ Engine/Resource/*    │ Shared     │ ✅✅ 90%        │ 낮음 (약간 리팩토링)   │
│ Engine/Renderer/*    │ Shared     │ ✅ 70%         │ 중간 (선택적)         │
│ Engine/Application   │ Shared     │ ❌ 10%         │ 높음 (재구현 필요)     │
│ Editor/*             │ Executable │ ❌ 5%          │ 해당없음 (제외)        │
└──────────────────────┴────────────┴─────────────────┴──────────────────────┘
```

### 2.2 모듈별 상세 분석

#### A. Core 모듈 - ✅ 완전 재활용

**의존성**: 없음 (STL만 사용)

| 컴포넌트 | 상태 | 설명 |
|---------|------|------|
| Log.h | ✅ | 독립적인 로깅 시스템 |
| Hash.h | ✅ | 순수 해시 함수 (FNV-1a, Jenkins) |
| Math/Common.h | ✅ | GLM 래퍼 |
| Exception.h | ✅ | 커스텀 예외 시스템 |
| FileSystem.h | ✅ | 크로스 플랫폼 파일 I/O |
| Timer.h | ✅ | 성능 타이머 |
| Color.h | ✅ | 색상 유틸리티 |

**권장**: 수정 없이 그대로 추출

---

#### B. Platform 모듈 - ✅ 조건부 재활용

**의존성**: Core

| 컴포넌트 | 플랫폼 | 상태 |
|---------|--------|------|
| MacFileSystem.mm | macOS | ✅ |
| MacWindow.mm | macOS | ✅ |
| MacInput.mm | macOS | ✅ |
| MacSystemContext.mm | macOS | ✅ |
| WinFileSystem.cpp | Windows | ✅ |
| WinWindow.cpp | Windows | ✅ |
| NeonSimd.h | ARM64 | ✅ (선택) |
| SSESimd.h | x64 | ✅ (선택) |

**권장**: 타겟 OS에 맞게 필터링하여 추출

---

#### C. RHI 모듈 - ✅ 완전 재활용 (핵심)

**의존성**: Core, Platform

| 컴포넌트 | 상태 | 설명 |
|---------|------|------|
| RHIContext.h | ✅ | 그래픽 컨텍스트 팩토리 |
| RHIDefinition.h | ✅ | 모든 RHI 타입 정의 |
| RenderHandle.h | ✅ | 렌더패스, 프레임버퍼, 파이프라인 |
| ResourceHandle.h | ✅ | 버퍼, 텍스처, 셰이더 |
| CommandHandle.h | ✅ | 커맨드 버퍼 |
| Swapchain.h | ✅ | 스왑체인 추상화 |
| Metal/* | ✅ | Metal 구현 (~1,866 LOC) |
| Vulkan/* | ✅ | Vulkan 구현 (~4,048 LOC) |

**권장**: 수정 없이 그대로 추출. **경량 프로젝트의 핵심 모듈**

---

#### D. Engine/Resource 모듈 - ✅ 대부분 재활용

**의존성**: Core, RHI

| 컴포넌트 | 상태 | 설명 |
|---------|------|------|
| Object.h | ✅ | 기본 클래스 (레퍼런스 카운팅) |
| Image.h | ✅ | 이미지 데이터 컨테이너 |
| Mesh.h | ✅ | 메시 데이터 (버텍스, 인덱스) |
| Material.h | ✅ | 머티리얼 속성 |
| Shader.h | ⚠️ | 셰이더 래퍼 (일부 TODO 있음) |
| ObjectManager.h | ✅ | 에셋 로딩 (ASSIMP 의존) |

**외부 의존성**: ASSIMP (모델 로딩용)

**권장**: 대부분 그대로 추출. Shader.h는 TODO 정리 필요.

---

#### E. Engine/Renderer 모듈 - ⚠️ 선택적 재활용

**의존성**: Core, RHI, Resource

| 컴포넌트 | 상태 | 설명 |
|---------|------|------|
| RenderPath.h | ⚠️ | 기본 구조 좋음, Engine 커플링 있음 |
| RenderPass.h | ✅ | 추상 렌더패스 |
| RenderTarget.h | ⚠️ | 렌더 타겟 캐시 |
| ForwardPath.h | ✅ | 포워드 렌더링 |
| RendererDefinition.h | ✅ | 렌더러 타입 정의 |

**권장**: 단순 뷰어에는 불필요, 고급 렌더링 필요시 추출

---

#### F. 제외 대상

| 컴포넌트 | 이유 |
|---------|------|
| Editor/* | ImGui 의존, UI 전용 |
| Engine/Application.h | 엔진 아키텍처에 강하게 결합 |
| Engine/Window.h | Application에 의존 |
| Engine/EngineContext.h | 엔진 전용 컨텍스트 |
| Client/* | 엔진 클라이언트 진입점 |

---

## 3. 모듈 분리 설계

### 3.1 디렉토리 구조

```
HSMR/
├── Modules/                        # ← 독립 모듈 (DLL/dylib)
│   ├── Core/
│   │   ├── include/
│   │   │   └── HS/
│   │   │       └── Core/
│   │   │           ├── Log.h
│   │   │           ├── Hash.h
│   │   │           ├── Math.h
│   │   │           ├── FileSystem.h
│   │   │           └── ...
│   │   ├── src/
│   │   │   └── *.cpp
│   │   └── CMakeLists.txt
│   │
│   ├── Platform/
│   │   ├── include/
│   │   │   └── HS/
│   │   │       └── Platform/
│   │   │           ├── Window.h
│   │   │           ├── Input.h
│   │   │           └── SystemContext.h
│   │   ├── src/
│   │   │   ├── Mac/
│   │   │   └── Win/
│   │   └── CMakeLists.txt
│   │
│   ├── RHI/
│   │   ├── include/
│   │   │   └── HS/
│   │   │       └── RHI/
│   │   │           ├── RHIContext.h
│   │   │           ├── RHIDefinition.h
│   │   │           ├── Handles.h
│   │   │           └── Swapchain.h
│   │   ├── src/
│   │   │   ├── Metal/
│   │   │   └── Vulkan/
│   │   └── CMakeLists.txt
│   │
│   └── Resource/
│       ├── include/
│       │   └── HS/
│       │       └── Resource/
│       │           ├── Object.h
│       │           ├── Image.h
│       │           ├── Mesh.h
│       │           ├── Material.h
│       │           └── ObjectManager.h
│       ├── src/
│       └── CMakeLists.txt
│
├── Engine/                         # ← 엔진 전용 (HSMR에서만 사용)
│   ├── Application/
│   ├── Renderer/
│   └── ...
│
├── Editor/
├── Client/
└── CMakeLists.txt
```

### 3.2 의존성 그래프

```
┌─────────────────────────────────────────────────────────┐
│                    새 프로젝트 (경량 뷰어)                │
│                           │                             │
│                           ▼                             │
│  ┌──────────────────────────────────────────────────┐  │
│  │              HSMR Modules (DLL/dylib)             │  │
│  │                                                   │  │
│  │   ┌──────────┐                                   │  │
│  │   │ Resource │ ← ObjectManager, Mesh, Image      │  │
│  │   └────┬─────┘                                   │  │
│  │        │                                         │  │
│  │        ▼                                         │  │
│  │   ┌──────────┐                                   │  │
│  │   │   RHI    │ ← Metal/Vulkan 추상화             │  │
│  │   └────┬─────┘                                   │  │
│  │        │                                         │  │
│  │        ▼                                         │  │
│  │   ┌──────────┐                                   │  │
│  │   │ Platform │ ← Window, Input, FileSystem       │  │
│  │   └────┬─────┘                                   │  │
│  │        │                                         │  │
│  │        ▼                                         │  │
│  │   ┌──────────┐                                   │  │
│  │   │   Core   │ ← Log, Math, Hash                 │  │
│  │   └──────────┘                                   │  │
│  │                                                   │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

---

## 4. CMake 설정

### 4.1 모듈 CMakeLists.txt

```cmake
# Modules/Core/CMakeLists.txt
project(HS_Core)

add_library(HS_Core STATIC
    src/Log.cpp
    src/Hash.cpp
    src/FileSystem.cpp
    src/Timer.cpp
)

target_include_directories(HS_Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(HS_Core PUBLIC cxx_std_17)

# 설치 규칙
install(TARGETS HS_Core EXPORT HSModulesTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY include/ DESTINATION include)
```

```cmake
# Modules/RHI/CMakeLists.txt
project(HS_RHI)

set(RHI_SOURCES
    src/RHIContext.cpp
)

if(APPLE)
    list(APPEND RHI_SOURCES
        src/Metal/MetalContext.mm
        src/Metal/MetalUtility.mm
        src/Metal/MetalCommandHandle.mm
    )
endif()

if(WIN32)
    list(APPEND RHI_SOURCES
        src/Vulkan/VulkanContext.cpp
        src/Vulkan/VulkanUtility.cpp
        src/Vulkan/VulkanDevice.cpp
    )
endif()

add_library(HS_RHI SHARED ${RHI_SOURCES})

target_link_libraries(HS_RHI
    PUBLIC HS_Core HS_Platform
)

if(APPLE)
    target_link_libraries(HS_RHI
        PRIVATE "-framework Metal"
        PRIVATE "-framework QuartzCore"
    )
endif()

if(WIN32)
    find_package(Vulkan REQUIRED)
    target_link_libraries(HS_RHI PRIVATE Vulkan::Vulkan)
endif()
```

### 4.2 새 프로젝트에서 사용

```cmake
# 새 프로젝트/CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
project(ModelViewer)

# HSMR 모듈 경로
set(HSMR_MODULES_DIR "/path/to/HSMR/Modules")

# 모듈 추가
add_subdirectory(${HSMR_MODULES_DIR}/Core ${CMAKE_BINARY_DIR}/HS_Core)
add_subdirectory(${HSMR_MODULES_DIR}/Platform ${CMAKE_BINARY_DIR}/HS_Platform)
add_subdirectory(${HSMR_MODULES_DIR}/RHI ${CMAKE_BINARY_DIR}/HS_RHI)
add_subdirectory(${HSMR_MODULES_DIR}/Resource ${CMAKE_BINARY_DIR}/HS_Resource)

# 뷰어 앱
add_executable(ModelViewer
    src/main.cpp
    src/App.cpp
    src/Viewer.cpp
)

target_link_libraries(ModelViewer PRIVATE
    HS_Core
    HS_Platform
    HS_RHI
    HS_Resource
)
```

---

## 5. 경량 모델 뷰어 예시

### 5.1 최소 구현 (~200 LOC)

```cpp
// 새 프로젝트/src/main.cpp
#include <HS/Core/Log.h>
#include <HS/Platform/Window.h>
#include <HS/RHI/RHIContext.h>
#include <HS/Resource/ObjectManager.h>
#include <HS/Resource/Mesh.h>

// 경량 App 클래스 (새 프로젝트에서 정의)
#include "App.h"

int main(int argc, char* argv[]) {
    // HSMR 모듈 초기화
    HS::Log::Initialize();

    App app({
        .title = "Model Viewer",
        .width = 1280,
        .height = 720,
        .api = HS::ERHIPlatform::METAL
    });

    // HSMR의 ObjectManager로 모델 로드
    auto mesh = HS::ObjectManager::LoadMeshFromFile("Models/bunny.obj");

    // HSMR의 RHI로 GPU 리소스 생성
    auto* rhi = app.GetRHI();
    auto* vertexBuffer = rhi->CreateBuffer("vbo",
        mesh->GetVertices().data(),
        mesh->GetVertexCount() * sizeof(Vertex),
        HS::EBufferUsage::VERTEX,
        HS::EBufferMemoryOption::STATIC
    );

    // 셰이더, 파이프라인 생성...
    auto* vs = HS::ObjectManager::LoadShaderFromFile("Shaders/model.vert.slang",
                                                      HS::EShaderStage::VERTEX, "main");
    auto* fs = HS::ObjectManager::LoadShaderFromFile("Shaders/model.frag.slang",
                                                      HS::EShaderStage::FRAGMENT, "main");

    // 메인 루프
    app.Run(
        [&](float dt) {
            // 카메라 업데이트
        },
        [&](HS::RHICommandBuffer* cmd) {
            cmd->BeginRenderPass(...);
            cmd->BindPipeline(pipeline);
            cmd->BindVertexBuffer(vertexBuffer, 0);
            cmd->Draw(mesh->GetVertexCount(), 0);
            cmd->EndRenderPass();
        }
    );

    // 정리
    rhi->DestroyBuffer(vertexBuffer);

    return 0;
}
```

### 5.2 App 클래스 (새 프로젝트에서 구현)

```cpp
// 새 프로젝트/src/App.h
#pragma once
#include <HS/RHI/RHIContext.h>
#include <HS/RHI/Swapchain.h>
#include <functional>

struct AppConfig {
    const char* title = "App";
    uint32_t width = 1280;
    uint32_t height = 720;
    HS::ERHIPlatform api = HS::ERHIPlatform::METAL;
};

class App {
public:
    App(const AppConfig& config);
    ~App();

    void Run(
        std::function<void(float)> onUpdate,
        std::function<void(HS::RHICommandBuffer*)> onRender
    );

    HS::RHIContext* GetRHI() { return _rhi; }
    HS::Swapchain* GetSwapchain() { return _swapchain; }

private:
    HS::RHIContext* _rhi;
    HS::Swapchain* _swapchain;
    void* _nativeWindow;  // 플랫폼별
    bool _running = true;
};
```

---

## 6. 마이그레이션 단계

### Phase 1: 모듈 구조 생성 (1-2일)

1. `Modules/` 디렉토리 생성
2. 기존 `Source/Core/`를 `Modules/Core/`로 복사
3. include 경로 재구성 (`HS/Core/` 형태)
4. CMakeLists.txt 작성

### Phase 2: RHI 모듈화 (2-3일)

1. `Source/RHI/`를 `Modules/RHI/`로 복사
2. Platform 의존성 정리
3. Metal/Vulkan 조건부 빌드 설정
4. 테스트

### Phase 3: Resource 모듈화 (1-2일)

1. `Engine/Resource/`에서 필요한 파일 추출
2. Object, Image, Mesh, Material, ObjectManager
3. ASSIMP 의존성 설정

### Phase 4: 검증 (1일)

1. HSMR 엔진이 Modules/를 링크하여 빌드되는지 확인
2. 새 프로젝트에서 Modules/만 링크하여 동작하는지 확인

---

## 7. 예상 결과

### 모듈 크기

| 모듈 | 예상 LOC | 빌드 타입 |
|------|---------|----------|
| HS_Core | ~1,500 | Static |
| HS_Platform | ~1,800 | Static |
| HS_RHI | ~6,000 | Shared |
| HS_Resource | ~2,000 | Shared |
| **합계** | **~11,300** | |

### 새 프로젝트 구조

```
ModelViewer/
├── src/
│   ├── main.cpp          # 진입점
│   ├── App.h/cpp         # 경량 앱 래퍼 (~200 LOC)
│   └── Viewer.h/cpp      # 뷰어 로직 (~300 LOC)
├── Shaders/
│   ├── model.vert.slang
│   └── model.frag.slang
├── CMakeLists.txt
└── (HSMR Modules 링크)

새 프로젝트 자체 코드: ~500 LOC
+ HSMR 모듈 링크: ~11,300 LOC (DLL)
```

---

## 8. 장점 요약

| 관점 | 이점 |
|------|------|
| **HSMR 프로젝트** | 엔진 구조 유지, 모듈화로 코드 품질 향상 |
| **새 프로젝트** | 검증된 RHI/Resource 즉시 사용 |
| **유지보수** | 모듈별 독립 테스트/업데이트 가능 |
| **학습** | Vulkan/Metal 코드가 재사용 가능한 형태로 정리 |
| **확장성** | 다른 프로젝트에서도 모듈 재사용 가능 |

---

---

## 9. Git Submodule 설정 가이드

### 9.1 모듈을 별도 저장소로 분리

```bash
# HSMR 프로젝트 루트에서
cd /path/to/HSMR

# Modules 폴더를 별도 저장소로 초기화
cd Modules
git init
git add .
git commit -m "Initial commit: Extract HS modules from HSMR"

# GitHub 등에 새 저장소 생성 후 push
git remote add origin git@github.com:username/HSModules.git
git push -u origin main
```

### 9.2 HSMR에서 서브모듈로 참조

```bash
# HSMR 프로젝트에서 기존 Modules 폴더 제거
cd /path/to/HSMR
rm -rf Modules

# 서브모듈로 다시 추가
git submodule add git@github.com:username/HSModules.git Modules

# 커밋
git add .gitmodules Modules
git commit -m "feat: Convert Modules to git submodule"
```

### 9.3 새 프로젝트에서 서브모듈 사용

```bash
# 새 프로젝트 생성
mkdir MyModelViewer && cd MyModelViewer
git init

# HS Modules 서브모듈 추가
git submodule add git@github.com:username/HSModules.git Modules/HS

# CMakeLists.txt 작성
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.22)
project(MyModelViewer)

# HS Modules 추가
add_subdirectory(Modules/HS ${CMAKE_BINARY_DIR}/HSModules)

# 앱 빌드
add_executable(MyModelViewer src/main.cpp)
target_link_libraries(MyModelViewer PRIVATE HS::Core HS::RHI)
EOF
```

### 9.4 서브모듈 업데이트

```bash
# 서브모듈 최신 버전으로 업데이트
git submodule update --remote Modules

# 또는 서브모듈 디렉토리에서 직접
cd Modules
git pull origin main
cd ..
git add Modules
git commit -m "Update HS Modules to latest"
```

### 9.5 서브모듈 포함하여 클론

```bash
# 새로 클론할 때 서브모듈 함께 가져오기
git clone --recurse-submodules git@github.com:username/MyProject.git

# 또는 클론 후 서브모듈 초기화
git clone git@github.com:username/MyProject.git
cd MyProject
git submodule init
git submodule update
```

---

## 10. 구현 완료 상태

### 10.1 완료된 작업

| 작업 | 상태 | 설명 |
|------|------|------|
| Modules/ 디렉토리 구조 | ✅ 완료 | Core, Platform, RHI, Resource |
| HSConfig.h 공통 설정 | ✅ 완료 | 타입, 매크로, Ref<T> 래퍼 포함 |
| Core CMakeLists.txt | ✅ 완료 | GLM 포함, 독립 빌드 가능 |
| Platform CMakeLists.txt | ✅ 완료 | Mac/Win/SIMD 조건부 빌드 |
| RHI CMakeLists.txt | ✅ 완료 | Metal/Vulkan 조건부 빌드 |
| Resource CMakeLists.txt | ✅ 완료 | ASSIMP 의존성 설정 |
| 소스 파일 복사 | ✅ 완료 | 모든 헤더/소스 복사됨 |
| README.md | ✅ 완료 | 사용법 문서화 |

### 10.2 추가 작업 필요

| 작업 | 우선순위 | 설명 |
|------|----------|------|
| Include 경로 업데이트 | 높음 | `"Precompile.h"` → `<HSConfig.h>` 등 |
| 빌드 테스트 | 높음 | 독립 빌드 검증 |
| HSMR에서 Modules 링크 | 중간 | 기존 빌드와 통합 |

---

*이 문서는 HSMR 프로젝트의 모듈 분리 및 경량 프로젝트 재활용 전략을 정의합니다.*
