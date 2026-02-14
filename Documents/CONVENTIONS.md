# HSMR 코딩 컨벤션 가이드

> **HSMR (High-Speed Modular Renderer)** 프로젝트의 코딩 컨벤션 문서입니다.
> 새로운 팀 멤버나 AI 에이전트가 프로젝트에 참여할 때 참고할 수 있도록 작성되었습니다.

---

## 목차

1. [프로젝트 개요](#1-프로젝트-개요)
2. [파일 및 디렉토리 구조](#2-파일-및-디렉토리-구조)
3. [네임스페이스](#3-네임스페이스)
4. [네이밍 규칙](#4-네이밍-규칙)
5. [코드 포매팅](#5-코드-포매팅)
6. [클래스 설계 패턴](#6-클래스-설계-패턴)
7. [메모리 관리](#7-메모리-관리)
8. [열거형(Enum) 규칙](#8-열거형enum-규칙)
9. [매크로 규칙](#9-매크로-규칙)
10. [플랫폼 추상화 패턴](#10-플랫폼-추상화-패턴)
11. [빌드 시스템 (CMake)](#11-빌드-시스템-cmake)
12. [주석 스타일](#12-주석-스타일)
13. [빠른 참조 테이블](#13-빠른-참조-테이블)

---

## 1. 프로젝트 개요

HSMR은 C++20 기반의 크로스 플랫폼 그래픽스 엔진입니다. Windows에서는 Vulkan, macOS에서는 Metal을 렌더링 백엔드로 사용하며, 모듈화된 계층 구조 아키텍처를 따릅니다.

### 모듈 의존성 계층

```
Foundation Layer:    Platform -> Core
Resource Layer:      Object -> ShaderSystem
Graphics Pipeline:   RHI -> Renderer -> Engine
Development Tools:   Editor (ImGui) -> Client
Specialized Systems: ECS, Physics, Geometry, Animation
```

각 레이어는 하위 레이어에만 의존해야 하며, 역방향 의존은 허용되지 않습니다.

### 기술 스택

| 항목 | 사양 |
|:-----|:-----|
| C++ 표준 | C++20 (필수) |
| RTTI | 비활성화 (`-fno-rtti`) |
| 플랫폼 매크로 | `__APPLE__` / `__WINDOWS__`, `__ARM64__` / `__X64__` |
| 빌드 시스템 | CMake 3.22.0+ |
| ARC | 비활성화 (C++/ObjC 혼합 코드) |

---

## 2. 파일 및 디렉토리 구조

### 전체 디렉토리 레이아웃

```
Source/
|-- Precompile.h              # 전역 타입 정의, 매크로, 플랫폼 감지
|-- Platform/                  # 하드웨어 추상화 레이어
|   |-- Mac/Private/           # macOS 구현 (.mm)
|   |-- Win/Private/           # Windows 구현 (.cpp)
|   +-- SDL/Private/           # SDL 크로스 플랫폼 구현
|-- Core/                      # 기반 유틸리티
|   |-- *.h                    # 모듈 루트에 공개 헤더
|   |-- Private/               # 구현 파일
|   |-- Native/                # Window/Event 추상화
|   |-- HAL/                   # Input, FileSystem, Timer
|   |-- Math/                  # 수학 라이브러리
|   +-- Memory/                # 메모리 관리
|-- RHI/                       # 렌더링 하드웨어 인터페이스
|   |-- Metal/Private/         # Metal 백엔드 (.mm)
|   |-- Vulkan/Private/        # Vulkan 백엔드 (.cpp)
|   +-- *.h                    # 크로스 플랫폼 RHI 헤더
|-- ShaderSystem/              # Slang/SPIRV-Cross 셰이더 컴파일
|-- Engine/                    # 핵심 엔진 시스템
|   |-- Resource/              # Object, Mesh, Material, Image, Shader
|   |-- Renderer/              # RenderPass, RenderResourceManager
|   +-- Scene/                 # Components (Light, Camera)
|-- Editor/                    # ImGui 기반 에디터 도구
|   |-- Core/                  # EditorWindow, EditorCamera, EditorContext
|   |-- Panel/                 # HierarchyPanel, ScenePanel, ResourcePanel
|   +-- Asset/                 # 에셋 관리
|-- Application/               # 애플리케이션 라이프사이클, GizmoController
+-- Client/                    # 실행 파일 진입점
```

### 파일 이름: PascalCase

모든 소스 파일은 PascalCase를 사용합니다. 하나의 파일에는 일반적으로 하나의 주요 클래스 또는 구조체가 포함됩니다.

| 파일 종류 | 확장자 | 예시 |
|:----------|:-------|:-----|
| 헤더 | `.h` | `SystemContext.h`, `RHIDefinition.h`, `MacWindow.h` |
| C++ 소스 | `.cpp` | `SystemContext.cpp`, `ShaderCompiler.cpp` |
| Objective-C++ | `.mm` | `MacWindow.mm`, `MetalContext.mm`, `MetalUtility.mm` |

### 공개 헤더와 구현 파일의 분리

이 프로젝트는 **공개 인터페이스**와 **구현 세부사항**을 물리적으로 분리합니다.

- **공개 헤더**: 모듈 루트 디렉토리에 배치
  - 예: `Source/Core/Log.h`, `Source/RHI/RHIDefinition.h`
- **구현 파일**: `Private/` 하위 디렉토리에 배치
  - 예: `Source/Core/Private/Log.cpp`, `Source/RHI/Metal/Private/MetalContext.mm`
- **플랫폼 전용 코드**: 플랫폼별 폴더에 배치
  - 예: `Source/RHI/Metal/`, `Source/RHI/Vulkan/`, `Source/Platform/Mac/`

이 구조를 통해 모듈의 공개 API를 빠르게 파악할 수 있고, 구현 변경이 다른 모듈에 영향을 주지 않습니다.

### 헤더 가드

`#pragma once` 대신 전통적인 `#ifndef` 기반 헤더 가드를 사용합니다.

**형식**: `__HS_[MODULE]_[FILENAME]_H__`

```cpp
// Source/Core/Log.h
#ifndef __HS_LOG_H__
#define __HS_LOG_H__

// ... 내용 ...

#endif /* __HS_LOG_H__ */
```

```cpp
// Source/RHI/RHIDefinition.h
#ifndef __HS_RHI_DEFINITION_H__
#define __HS_RHI_DEFINITION_H__

// ... 내용 ...

#endif
```

추가 예시: `__HS_PRECOMPILE_H__`, `__HS_SYSTEM_CONTEXT_H__`, `__HS_COLOR_H__`

### 인클루드 순서

인클루드는 다음 순서를 따릅니다. 각 그룹 사이에 빈 줄을 넣는 것을 권장합니다.

1. **자신의 헤더** (.cpp 파일인 경우)
2. **표준 라이브러리 헤더** (`<cstdint>`, `<vector>` 등)
3. **서드파티 헤더** (`<SDL3/SDL.h>`, `<Metal/Metal.h>` 등)
4. **프로젝트 헤더** (`"Core/Log.h"`, `"RHI/RHIDefinition.h"` 등)

```cpp
// Log.cpp 예시
#include "Core/Log.h"

#include <cstdio>
#include <cstdarg>
#include <stdexcept>

#include "Core/Exception.h"
```

---

## 3. 네임스페이스

### 기본 네임스페이스: `hs`

모든 프로젝트 코드는 반드시 네임스페이스 매크로 내부에 작성해야 합니다.

```cpp
// 엔진/코어 코드
HS_NS_BEGIN

class HS_API Camera
{
    // ...
};

HS_NS_END
```

### 에디터 네임스페이스: `hs::editor`

에디터 관련 코드는 중첩된 `editor` 네임스페이스를 사용합니다.

```cpp
HS_NS_EDITOR_BEGIN

class HS_EDITOR_API EditorWindow
{
    // ...
};

HS_NS_EDITOR_END
```

### 전방 선언(Forward Declaration)

네임스페이스를 직접 사용하여 전방 선언합니다. 매크로를 사용하지 않습니다.

```cpp
namespace hs { class Swapchain; }
namespace hs { struct NativeWindow; }
```

### 매크로 정의 (Precompile.h)

```cpp
#define HS_NS_BEGIN       namespace hs {
#define HS_NS_END         }

#define HS_NS_EDITOR_BEGIN namespace hs { namespace editor {
#define HS_NS_EDITOR_END   } }
```

---

## 4. 네이밍 규칙

### 클래스 및 구조체: PascalCase

```cpp
class SystemContext;     // 시스템 컨텍스트
class RHIContext;        // RHI 공통 인터페이스
class MetalContext;      // Metal 백엔드 구현
class EditorWindow;      // 에디터 윈도우
```

**구성 정보 구조체**는 `Info` 접미사를 사용합니다.

```cpp
struct TextureInfo;      // 텍스처 생성 정보
struct BufferInfo;       // 버퍼 생성 정보
struct SamplerInfo;      // 샘플러 생성 정보
struct ShaderInfo;       // 셰이더 생성 정보
struct RenderPassInfo;   // 렌더 패스 생성 정보
```

**RHI 핸들 타입**은 `RHI` 접두사를 사용합니다.

```cpp
struct RHITexture;
struct RHIBuffer;
struct RHIShader;
class  RHIRenderPass;
```

### 열거형 타입: E 접두사 + PascalCase

```cpp
enum class EPixelFormat;     // 픽셀 포맷
enum class EShaderStage;     // 셰이더 스테이지
enum class EWindowFlags;     // 윈도우 플래그
enum class ETextureUsage;    // 텍스처 용도
```

**예외**: 클래스 내부에 중첩된 열거형은 `E` 접두사를 생략합니다.

```cpp
class RHIHandle
{
public:
    enum class EType { Swapchain, Buffer, Texture, /* ... */ };
};

class Log
{
public:
    enum class EType { Info, Debug, Warning, Error, Crash, Assert };
};
```

### 열거형 값: PascalCase

```cpp
enum class EShaderStage
{
    None     = 0x00000000,
    Vertex   = 0x00000001,
    Fragment = 0x00000010,
    Compute  = 0x00000020,
};

enum class EAddressMode
{
    Invalid = 0,
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
};
```

### 함수: PascalCase

**공개(Public) 함수**는 PascalCase를 사용합니다.

```cpp
// 라이프사이클 함수
void Initialize();
void Finalize();
void Setup();
void Cleanup();

// Getter / Setter
glm::vec3 GetPosition() const;
void SetPosition(const glm::vec3& position);
bool IsValid() const;
bool HasTexture() const;

// 팩토리 함수
bool CreateNativeWindow(/* ... */);
void DestroyBuffer(RHIBuffer* buffer);
Image* LoadImage(const char* path);
```

**비공개(Private) 메서드**는 `_camelCase` (밑줄 접두사 + camelCase)를 사용합니다.

```cpp
private:
    void _setupPanels();
    void _drawEntityNode(Entity* entity);
    void _updateSceneCamera();
    void _processShortcuts();
```

**플랫폼 내부 함수**는 `Internal` 접미사를 사용합니다.

```cpp
// 공개 API 선언
bool CreateNativeWindow(const char* name, uint16 width, uint16 height,
                        EWindowFlags flag, NativeWindow& outNativeWindow);

// 플랫폼별 내부 구현 (전방 선언)
bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height,
                                EWindowFlags flag, NativeWindow& outNativeWindow);
```

### 멤버 변수: `_camelCase` (밑줄 접두사)

모든 `private`/`protected` 멤버 변수는 밑줄 접두사를 사용합니다.

```cpp
class Camera
{
private:
    glm::vec3 _position;
    glm::vec3 _rotation;
    float     _fov;
    bool      _isValid;
    Scoped<RenderPath> _renderer;
};
```

### 정적 멤버 변수: `s_` 접두사

```cpp
class Input
{
    static SDL_Window* s_sdlWindow;
    static bool        s_button[256];
    static Input*      s_instance;
};
```

### 지역 변수: camelCase (접두사 없음)

```cpp
float shininess = 32.0f;
uint32 vertexCount = mesh->GetVertexCount();
std::string path = "assets/textures/diffuse.png";
```

### 함수 매개변수: camelCase

```cpp
void SetViewport(float x, float y, float width, float height);
bool CreateTexture(const TextureInfo& info, const char* name);
```

**출력 매개변수**는 `out` 접두사를 사용합니다.

```cpp
bool CreateNativeWindow(const char* name, uint16 width, uint16 height,
                        EWindowFlags flag, NativeWindow& outNativeWindow);
void GetExtent(uint32& outWidth, uint32& outHeight);
bool ExtractReflection(ShaderReflection& outReflection);
```

### 상수 및 매크로: `HS_` 접두사 + SCREAMING_SNAKE_CASE

```cpp
#define HS_BIT(x)                 ((uint64)1 << (x))
#define HS_FORCEINLINE            inline __attribute__((always_inline))
#define HS_DEBUG_BREAK()          __builtin_trap()
#define HS_CHAR_INIT_LENGTH       512
#define HS_INT32_MAX              (2147483647)
#define HS_FLT_MAX                (3.402823466e+38F)
#define HS_STRINGIFY(x)           #x
#define HS_TO_STRING(x)           HS_STRINGIFY(x)
```

### 타입 별칭: Precompile.h에 정의

이 프로젝트는 고정 크기 정수 타입에 대해 짧은 별칭을 사용합니다.

```cpp
typedef int8_t   int8;
typedef uint8_t  uint8;
typedef int16_t  int16;
typedef uint16_t uint16;
typedef int32_t  int32;
typedef uint32_t uint32;
typedef int64_t  int64;
typedef uint64_t uint64;
```

스마트 포인터 별칭:

```cpp
template <typename Tp>
using Scoped = std::unique_ptr<Tp>;

template <typename Tp, typename... Args>
constexpr Scoped<Tp> MakeScoped(Args&&... args)
{
    return std::make_unique<Tp>(std::forward<Args>(args)...);
}
```

코드에서는 `std::unique_ptr` 대신 항상 `Scoped<T>`와 `MakeScoped<T>()`를 사용합니다.

```cpp
// 올바른 사용
Scoped<EditorCamera> _editorCamera;
auto camera = MakeScoped<EditorCamera>();

// 지양하는 사용
std::unique_ptr<EditorCamera> _editorCamera;  // Scoped<T>를 사용할 것
```

---

## 5. 코드 포매팅

### 들여쓰기: 스페이스 4칸

탭이 아닌 **스페이스 4칸**을 사용합니다. IDE 설정을 확인하세요.

### 중괄호 스타일: Allman (다음 줄 중괄호)

여는 중괄호를 항상 새로운 줄에 배치합니다.

```cpp
void Foo()
{
    if (condition)
    {
        // ...
    }
    else
    {
        // ...
    }
}

class HS_API Camera
{
public:
    Camera();
    ~Camera() = default;

private:
    glm::vec3 _position;
};

enum class EType
{
    Unknown,
    Mesh,
    Material,
};
```

**한 줄 본문**이라도 중괄호를 생략하지 않는 것을 권장합니다.

```cpp
// 권장
if (ptr == nullptr)
{
    return false;
}

// 허용되지만 권장하지 않음
if (ptr == nullptr)
    return false;
```

### 정렬

관련된 선언이 여러 줄에 걸쳐 있을 때, 가독성을 위해 수직 정렬합니다.

**멤버 변수 대입 정렬**:

```cpp
outNativeWindow.handle        = s_sdlWindow;
outNativeWindow.graphicsView  = s_metalView;
outNativeWindow.graphicsLayer = (__bridge void*)layer;
outNativeWindow.width         = width;
outNativeWindow.height        = height;
```

**switch-case 정렬**:

```cpp
case SDL_SCANCODE_A:         return static_cast<uint8>(Button::A);
case SDL_SCANCODE_B:         return static_cast<uint8>(Button::B);
case SDL_SCANCODE_BACKSPACE: return static_cast<uint8>(Button::Back);
```

**열거형 값 정렬**:

```cpp
Fullscreen = HS_BIT(0),
Opengl     = HS_BIT(1),
Hidden     = HS_BIT(3),
Resizable  = HS_BIT(5),
```

**구조체 멤버 정렬**:

```cpp
struct Attachment
{
    EPixelFormat  format;
    ELoadAction   loadAction;
    EStoreAction  storeAction;
    ClearValue    clearValue;
    uint8         sampleCount;
    bool          isDepthStencil = false;
};
```

### 공백 규칙

| 규칙 | 올바른 예시 | 잘못된 예시 |
|:-----|:-----------|:-----------|
| 키워드 뒤 공백 | `if (...)`, `for (...)`, `while (...)` | `if(...)`, `for(...)` |
| 이항 연산자 양쪽 공백 | `x = y + z`, `lhs \| rhs` | `x=y+z` |
| 함수 호출 괄호 앞 공백 없음 | `GetType()`, `SetPosition(pos)` | `GetType ()` |
| 쉼표 뒤 공백 | `func(a, b, c)` | `func(a,b,c)` |

### 줄 길이

약 **120자**를 소프트 리밋으로 합니다. 초과 시 적절히 줄 바꿈합니다.

---

## 6. 클래스 설계 패턴

### API 가시성 매크로

외부에 공개되는 모든 클래스와 구조체는 해당 모듈의 API 매크로를 사용합니다.

```cpp
class HS_API Camera { /* ... */ };                      // 엔진 모듈
class HS_EDITOR_API EditorWindow { /* ... */ };          // 에디터 모듈
struct HS_APPLICATION_API Transform { /* ... */ };       // 애플리케이션 모듈
class HS_SHADER_SYSTEM_API ShaderCache { /* ... */ };    // 셰이더 시스템 모듈
```

각 모듈의 API 매크로는 `Precompile.h`에 정의되어 있으며, macOS에서는 `__attribute__((__visibility__("default")))`, Windows에서는 `__declspec(dllexport/dllimport)`으로 확장됩니다.

### 생성자: 멤버 초기화 리스트 선호

```cpp
Material()
    : Object(EType::Material)
    , _shader(nullptr)
{
}

Image(void* data, uint32 width, uint32 height, uint32 channel) noexcept
    : Object(EType::Image)
    , _width(width)
    , _height(height)
    , _channel(channel)
{
    // 본문
}
```

멤버 초기화 리스트 각 항목은 `, `(쉼표)를 줄의 **시작**에 배치합니다. 이렇게 하면 항목 추가/제거 시 diff가 깔끔해집니다.

### 인라인 Getter/Setter: `HS_FORCEINLINE` 사용

간단한 접근자 함수는 헤더에 인라인으로 정의합니다.

```cpp
HS_FORCEINLINE glm::vec3 GetPosition() const { return _position; }
HS_FORCEINLINE void SetPosition(const glm::vec3& position) { _position = position; }
HS_FORCEINLINE bool IsValid() const { return _refs > 0; }
HS_FORCEINLINE int GetRefCount() const { return _refs; }
```

### 가상 함수와 override

- 기본 클래스는 **가상 소멸자**를 가져야 합니다.
- 파생 클래스에서 오버라이드하는 함수는 반드시 `override` 키워드를 사용합니다.
- 순수 가상 함수는 `= 0`을 사용합니다.
- 더 이상 상속되지 않는 클래스는 `final`을 사용할 수 있습니다.

```cpp
// 기본 클래스
class HS_API RHIContext
{
public:
    virtual ~RHIContext() = default;
    virtual bool Initialize() = 0;
    virtual void Finalize() = 0;
};

// 파생 클래스
class HS_API MetalContext : public RHIContext
{
public:
    ~MetalContext() override;
    bool Initialize() override;
    void Finalize() override;
};
```

### noexcept

이동 연산과 간단한 접근자에는 `noexcept`를 명시합니다.

```cpp
Color(Color&& o) noexcept : _data(o._data) {}
Image(Image&& o) noexcept;
HS_FORCEINLINE float& R() noexcept { return _data.r; }
```

### 접근 지정자 순서

클래스 내부에서 접근 지정자는 다음 순서를 따릅니다.

```cpp
class HS_API MyClass
{
public:
    // 생성자 / 소멸자
    // 공개 메서드
    // 공개 멤버 (가능한 최소화)

protected:
    // 보호된 메서드
    // 보호된 멤버

private:
    // 비공개 메서드 (_camelCase)
    // 비공개 멤버 (_camelCase)
};
```

---

## 7. 메모리 관리

### 스마트 포인터 (권장)

새로 작성하는 코드에서는 소유권이 명확한 경우 `Scoped<T>`를 사용합니다.

```cpp
Scoped<RenderPath> _renderer;
Scoped<EditorCamera> _editorCamera;
auto camera = MakeScoped<EditorCamera>();
```

### 원시 포인터 (특정 상황에서 사용)

다음과 같은 경우에는 원시 포인터를 사용합니다.

- **플랫폼 핸들**: Metal/Vulkan의 네이티브 핸들
- **RHI 리소스**: RHI 시스템이 수명을 관리하는 리소스
- **ObjectManager가 관리하는 객체**: Proxy 패턴으로 관리되는 리소스

```cpp
void* _device;           // 플랫폼 핸들 (MTLDevice*, VkDevice)
RHITexture* texture;     // RHI 리소스 포인터
Shader* _shader;         // ObjectManager가 관리
```

### 참조 카운팅 (RHI 리소스)

`RHIHandle`을 상속하는 RHI 리소스는 참조 카운팅으로 수명을 관리합니다.

```cpp
class RHIHandle
{
public:
    HS_FORCEINLINE int Retain()
    {
        return ++_refs;
    }

    HS_FORCEINLINE int Release()
    {
        HS_ASSERT(_refs > 0, "Over Released!");
        if (--_refs == 0)
        {
            delete this;
            return 0;
        }
        return _refs;
    }

protected:
    int _refs = 1;  // 생성 시 참조 카운트 1로 시작
};
```

### 원칙 정리

| 소유권 유형 | 사용할 도구 | 예시 |
|:-----------|:-----------|:-----|
| 단독 소유 | `Scoped<T>` | 엔진 서브시스템, 에디터 컴포넌트 |
| 참조 카운팅 | `Retain()` / `Release()` | RHI 리소스 (`RHITexture`, `RHIBuffer`) |
| 비소유 참조 | 원시 포인터 | 플랫폼 핸들, 다른 시스템이 관리하는 객체 |

---

## 8. 열거형(Enum) 규칙

### 항상 `enum class` 사용

일반 `enum`이 아닌 **범위 지정 열거형**(`enum class`)을 사용합니다.

```cpp
// 올바른 사용
enum class EShaderStage
{
    None     = 0,
    Vertex   = 1,
    Fragment = 2,
    Compute  = 4,
};

// 지양하는 사용
enum EShaderStage  // enum class를 사용할 것
{
    SHADER_STAGE_NONE,
    SHADER_STAGE_VERTEX,
};
```

### 열거형 값: PascalCase

```cpp
enum class ELoadAction
{
    Invalid = 0,
    DontCare,
    Load,
    Clear,
};
```

### 픽셀 포맷 네이밍 패턴

채널 식별자(R, G, B, A, D, S)는 **대문자**를 유지하고, 나머지는 PascalCase를 따릅니다.

```cpp
enum class EPixelFormat
{
    R8Unorm = 10,
    RG8Unorm = 30,
    R8G8B8A8Unorm = 70,
    R8G8B8A8Srgb = 71,
    Rgba16f = 102,
    Rgba32f = 112,
    Depth32 = 252,
    Depth24Stencil8 = 255,
};
```

### 비트 플래그 열거형

비트 플래그를 사용하는 열거형에는 **기본 타입을 명시**하고, **비트 연산자 오버로드**를 함께 정의합니다.

```cpp
enum class ETextureUsage : uint16
{
    Unknown              = 0x0000,
    Static               = 0x0001,
    Staging              = 0x0002,
    Sampled              = 0x0004,
    Storage              = 0x0008,
    ColorAttachment      = 0x0010,
    DepthStencilAttachment = 0x0020,
};

// 반드시 제공해야 하는 연산자 오버로드
HS_FORCEINLINE ETextureUsage operator|(ETextureUsage lhs, ETextureUsage rhs)
{
    return static_cast<ETextureUsage>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs));
}

HS_FORCEINLINE ETextureUsage operator&(ETextureUsage lhs, ETextureUsage rhs)
{
    return static_cast<ETextureUsage>(static_cast<uint32>(lhs) & static_cast<uint32>(rhs));
}

HS_FORCEINLINE ETextureUsage operator|=(ETextureUsage& lhs, ETextureUsage rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

HS_FORCEINLINE ETextureUsage operator&=(ETextureUsage& lhs, ETextureUsage rhs)
{
    lhs = lhs & rhs;
    return lhs;
}
```

### 사용 예시

```cpp
ETextureUsage usage = ETextureUsage::Sampled | ETextureUsage::ColorAttachment;

if ((usage & ETextureUsage::Sampled) != 0)
{
    // 샘플링 가능한 텍스처
}
```

---

## 9. 매크로 규칙

### 로깅: `HS_LOG(symbol, fmt, ...)`

`printf` 스타일의 포맷 문자열을 사용합니다.

```cpp
HS_LOG(info, "Window created: %s (%dx%d)", name, width, height);
HS_LOG(debug, "Vertex count: %u", vertexCount);
HS_LOG(warning, "Resource not found: %s", path);
HS_LOG(error, "Failed to create buffer: %s", bufferName);
HS_LOG(crash, "Fatal error: %s", msg);  // HS_DEBUG_BREAK 트리거
```

| 심볼 | 용도 | 비고 |
|:-----|:-----|:-----|
| `info` | 일반 정보 출력 | |
| `debug` | 디버깅용 상세 정보 | |
| `warning` | 주의가 필요한 상황 | |
| `error` | 오류 발생 | |
| `crash` | 치명적 오류 | `HS_DEBUG_BREAK()` 호출 |

### 어서션

```cpp
// 조건 검사 + 실패 시 디버그 브레이크 (Debug 빌드만)
HS_ASSERT(ptr != nullptr, "Pointer must not be null");

// 결과 검사 (Debug 빌드만)
HS_CHECK(result, "Operation failed");

// 예외 발생 + 디버그 브레이크 (모든 빌드)
HS_THROW("Unrecoverable error: %s", detail);
```

`HS_ASSERT`와 `HS_CHECK`는 Debug 빌드에서만 활성화됩니다. Release 빌드에서는 빈 매크로로 확장됩니다. `HS_THROW`는 모든 빌드에서 활성화됩니다.

### 유틸리티 매크로

```cpp
HS_BIT(x)                    // 비트 플래그: (uint64)1 << x
HS_FORCEINLINE               // 강제 인라인
HS_FORCENOINLINE              // 인라인 금지
HS_STRINGIFY(x)              // 토큰을 문자열로 변환
HS_TO_STRING(x)              // 매크로 확장 후 문자열화
HS_DIR_SEPERATOR             // 플랫폼별 경로 구분자 ('/' 또는 '\\')
HS_CHAR_INIT_LENGTH          // 기본 문자열 버퍼 크기 (512)
HS_CHAR_INIT_SHORT_LENGTH    // 짧은 문자열 버퍼 크기 (256)
HS_CHAR_INIT_LONG_LENGTH     // 긴 문자열 버퍼 크기 (1024)
```

---

## 10. 플랫폼 추상화 패턴

### Internal 함수 패턴

크로스 플랫폼 코드는 공개 API와 플랫폼별 내부 구현을 분리합니다.

```
NativeWindow.h        (공개 API 선언)
    |
NativeWindow.cpp      (CreateNativeWindowInternal 전방 선언 + 호출)
    |
    +-- MacWindow.mm   (macOS 구현: CreateNativeWindowInternal)
    +-- WinWindow.cpp  (Windows 구현: CreateNativeWindowInternal)
```

```cpp
// NativeWindow.h - 공개 API
bool CreateNativeWindow(const char* name, uint16 width, uint16 height,
                        EWindowFlags flag, NativeWindow& outNativeWindow);

// NativeWindow.cpp - 내부 함수 전방 선언 및 위임
bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height,
                                EWindowFlags flag, NativeWindow& outNativeWindow);

bool CreateNativeWindow(const char* name, uint16 width, uint16 height,
                        EWindowFlags flag, NativeWindow& outNativeWindow)
{
    return CreateNativeWindowInternal(name, width, height, flag, outNativeWindow);
}

// MacWindow.mm - macOS 전용 구현
bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height,
                                EWindowFlags flag, NativeWindow& outNativeWindow)
{
    // Metal/Cocoa 기반 윈도우 생성
}
```

### 조건부 컴파일

```cpp
// 플랫폼 분기
#if defined(__APPLE__)
    // macOS / Metal 코드
#elif defined(__WINDOWS__)
    // Windows / Vulkan 코드
#endif

// SDL 사용 여부 분기
#ifdef __SDL__
    // SDL 경로
#else
    // 네이티브 경로
#endif

// 아키텍처 분기
#if defined(__ARM64__)
    // ARM64 (Apple Silicon)
#elif defined(__X64__)
    // x64 (Intel/AMD)
#endif
```

### RHI 패턴

RHI 모듈은 렌더링 API 차이를 추상화합니다. 공통 인터페이스는 `RHI/` 루트에, 플랫폼별 구현은 하위 디렉토리에 위치합니다.

```
Source/RHI/
|-- RHIDefinition.h        # 공통 타입 정의 (EPixelFormat, TextureInfo 등)
|-- RHIContext.h            # 공통 인터페이스 (virtual)
|-- Metal/
|   +-- Private/
|       |-- MetalContext.mm  # Metal 구현
|       +-- MetalUtility.mm  # Metal 유틸리티
+-- Vulkan/
    +-- Private/
        |-- VulkanContext.cpp # Vulkan 구현
        +-- VulkanUtility.cpp
```

---

## 11. 빌드 시스템 (CMake)

### 빌드 명령어

```bash
# 빌드 파일 생성 (프로젝트 루트에서)
cmake -S . -B Build

# 전체 빌드
cmake --build Build --config Debug
cmake --build Build --config Release

# 특정 타겟 빌드
cmake --build Build --target Client --config Debug

# CMake 재생성 (편의 타겟)
cmake --build Build --target RegenerateCMake
```

### 빌드 타입

| 타입 | 용도 |
|:-----|:-----|
| `Debug` | 개발 및 디버깅 (어서션 활성화, 최적화 없음) |
| `Release` | 배포용 (최적화 활성화) |
| `MinSizeRel` | 크기 최적화 |
| `RelWithDebInfo` | 디버그 정보를 포함한 릴리스 |

### 모듈 CMakeLists.txt 패턴

각 모듈은 일관된 CMake 패턴을 따릅니다.

```cmake
set(TARGET_NAME Core)
set(TOTAL_FILES)

# 공개 헤더
set(CORE_HEADERS Log.h Color.h Math.h ...)
source_group("Public" FILES ${CORE_HEADERS})
list(APPEND TOTAL_FILES ${CORE_HEADERS})

# 구현 파일
set(CORE_SOURCES Private/Log.cpp Private/Color.cpp ...)
source_group("Private" FILES ${CORE_SOURCES})
list(APPEND TOTAL_FILES ${CORE_SOURCES})

# 라이브러리 타겟 생성
add_library(${TARGET_NAME} STATIC ${TOTAL_FILES})
target_compile_definitions(${TARGET_NAME} PRIVATE HS_CORE HS_API_EXPORT)
```

### 주요 규칙

- `source_group()`을 사용하여 IDE의 프로젝트 탐색기에서 논리적으로 파일을 그룹화합니다.
- 모듈의 공개 여부에 따라 `STATIC` 또는 `SHARED` 라이브러리로 빌드합니다.
- `target_compile_definitions()`에 `HS_API_EXPORT`를 정의하여 DLL 내보내기를 활성화합니다.

---

## 12. 주석 스타일

### 파일 헤더

모든 소스 파일의 상단에 다음 형식의 헤더 주석을 작성합니다.

```cpp
//
//  Camera.h
//  Engine
//
//  Created by Yongsik Im on 5/16/2025
//
```

### 인라인 주석

코드는 가능한 한 **자기 설명적(self-documenting)**이어야 합니다. 주석은 "왜(why)"를 설명할 때 사용하고, "무엇을(what)"하는지는 코드 자체가 나타내도록 합니다.

```cpp
void* graphicsLayer; // CAMetalLayer* (Metal), nullptr (Vulkan)
float scale = 1.0f;

// 프레임마다 입력 상태 초기화
Input::s_move.isMoved = 0;
```

한국어와 영어 주석이 혼용됩니다. 플랫폼 관련 주의사항은 한국어로 작성되기도 합니다.

```cpp
uint32 binding; // Metal에서는 무시됩니다.
```

### 섹션 구분자: `#pragma region`

관련된 코드 블록을 논리적으로 구분할 때 `#pragma region`을 사용합니다.

```cpp
#pragma region ShaderInput
// 셰이더 입력 관련 코드
// ...
#pragma endregion
```

### 주석이 불필요한 경우

다음과 같은 코드에는 주석을 달지 않습니다. 이름만으로 의도가 명확하기 때문입니다.

```cpp
// 주석 불필요 - 함수 이름이 의도를 설명
bool IsValid() const { return _refs > 0; }
glm::vec3 GetPosition() const { return _position; }
void SetRotation(const glm::vec3& rotation) { _rotation = rotation; }
```

---

## 13. 빠른 참조 테이블

### 네이밍 규칙 요약

| 분류 | 규칙 | 예시 |
|:-----|:-----|:-----|
| 파일 이름 | PascalCase | `SystemContext.h`, `MetalContext.mm` |
| 헤더 가드 | `__HS_[MODULE]_[FILE]_H__` | `__HS_LOG_H__` |
| 클래스 / 구조체 | PascalCase | `EditorWindow`, `RHITexture` |
| 열거형 타입 | E 접두사 + PascalCase | `EPixelFormat`, `EShaderStage` |
| 열거형 값 | PascalCase | `Vertex`, `ClampToEdge`, `R8G8B8A8Unorm` |
| 공개 메서드 | PascalCase | `Initialize()`, `GetPosition()` |
| 비공개 메서드 | _camelCase | `_setupPanels()`, `_drawEntityNode()` |
| 멤버 변수 | _camelCase | `_position`, `_renderer` |
| 정적 변수 | s_ 접두사 | `s_instance`, `s_sdlWindow` |
| 지역 변수 | camelCase | `vertexCount`, `shininess` |
| 출력 매개변수 | out 접두사 | `outWidth`, `outNativeWindow` |
| 매크로 | HS_ + SCREAMING_SNAKE_CASE | `HS_LOG`, `HS_FORCEINLINE`, `HS_BIT` |
| 구성 구조체 | PascalCase + Info 접미사 | `TextureInfo`, `BufferInfo` |
| 네임스페이스 | `hs` / `hs::editor` | `HS_NS_BEGIN` / `HS_NS_EDITOR_BEGIN` |
| 스마트 포인터 | `Scoped<T>` | `Scoped<Camera> _camera;` |
| API 매크로 | 모듈별 | `HS_API`, `HS_EDITOR_API` |

### 포매팅 규칙 요약

| 항목 | 규칙 |
|:-----|:-----|
| 들여쓰기 | 스페이스 4칸 |
| 중괄호 | Allman 스타일 (다음 줄에 여는 중괄호) |
| 줄 길이 | ~120자 소프트 리밋 |
| 정렬 | 관련 선언의 수직 정렬 권장 |
| 빈 줄 | 논리적 섹션 사이에 1줄 |

### 파일 배치 규칙

| 파일 종류 | 위치 | 예시 |
|:----------|:-----|:-----|
| 공개 헤더 | 모듈 루트 | `Source/Core/Log.h` |
| 구현 파일 | `Private/` 하위 | `Source/Core/Private/Log.cpp` |
| 플랫폼 전용 | 플랫폼 디렉토리 | `Source/RHI/Metal/Private/MetalContext.mm` |
| 구성 구조체 | 관련 모듈 헤더 | `RHIDefinition.h` 내 `TextureInfo` |

### 자주 하는 실수와 주의사항

| 실수 | 올바른 방법 |
|:-----|:-----------|
| `std::unique_ptr` 직접 사용 | `Scoped<T>` 별칭 사용 |
| `#pragma once` 사용 | `#ifndef __HS_..._H__` 가드 사용 |
| 네임스페이스 직접 작성 | `HS_NS_BEGIN` / `HS_NS_END` 매크로 사용 |
| 일반 `enum` 사용 | `enum class` 사용 |
| 멤버 변수에 접두사 누락 | `_camelCase` 형식 준수 |
| `inline` 키워드 사용 | `HS_FORCEINLINE` 매크로 사용 |
| 탭 사용 | 스페이스 4칸 사용 |

---

## 변경 이력

| 날짜 | 내용 |
|:-----|:-----|
| 2025-02-14 | 초기 작성 - 코드베이스 분석 기반 컨벤션 문서화 |
