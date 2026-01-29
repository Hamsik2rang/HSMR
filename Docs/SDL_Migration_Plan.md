# HSMR SDL 마이그레이션 계획서

**브랜치**: `enginelet`
**백업**: `experimental/engine`
**목표**: 경량 프로토타이핑 렌더러 - SDL + Vulkan + 선택적 ImGui (Windows)

---

## 요약

| 항목 | 현재 | 변경 후 |
|------|------|---------|
| Platform | Win32 네이티브 | SDL3 |
| RHI | Vulkan | Vulkan (변경 없음) |
| GUI | ImGui (필수) | ImGui (선택적) |
| 타겟 플랫폼 | Windows | Windows |

---

## 1단계: CMake 설정

### Root CMakeLists.txt 수정

```cmake
# 새로운 옵션 추가 (line ~70 부근)
option(HSMR_USE_SDL "Use SDL3 for platform abstraction" ON)
option(HSMR_ENABLE_IMGUI "Enable ImGui integration" ON)

# SDL3 경로 설정
if(HSMR_USE_SDL)
    set(HS_SDL3_INCLUDE_DIR "${HS_DEPS_INCLUDE_DIR}/SDL3")
    set(HS_SDL3_LIB "${HS_DEPS_LIB_DIR}/SDL3.lib")
    set(HS_SDL3_DLL "${HS_DEPS_DLL_DIR}/SDL3.dll")
    include_directories(SYSTEM ${HS_SDL3_INCLUDE_DIR})
    add_compile_definitions(HSMR_USE_SDL)
endif()

if(HSMR_ENABLE_IMGUI)
    add_compile_definitions(HSMR_ENABLE_IMGUI)
endif()
```

### SDL3 의존성 추가

```
Dependency/
  include/SDL3/
    SDL.h, SDL_vulkan.h, ...
  lib/win/Debug/SDL3.lib
  lib/win/Release/SDL3.lib
  dll/win/Debug/SDL3.dll
  dll/win/Release/SDL3.dll
```

---

## 2단계: Platform 모듈 - SDL 구현

### 새 파일 생성

**`Source/Platform/SDL/SDLPlatform.h`**
```cpp
#pragma once
#include "Precompile.h"
#include "Core/Native/NativeWindow.h"

#ifdef HSMR_USE_SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

HS_NS_BEGIN

// SDL 윈도우 핸들 래퍼
SDL_Window* GetSDLWindow();

HS_NS_END
#endif
```

**`Source/Platform/SDL/Private/SDLPlatform.cpp`**

핵심 구현 내용:
- `CreateNativeWindowInternal()` - SDL_CreateWindow 사용
- `PollNativeEventInternal()` - SDL_PollEvent로 이벤트 처리
- `DestroyNativeWindowInternal()` - SDL_DestroyWindow + SDL_Quit
- SDL 이벤트 → NativeEvent 매핑 (WINDOW_RESIZE, KEY_DOWN 등)
- Input 시스템 연동 (SDL_Scancode → Input::Button 매핑)

### Platform/CMakeLists.txt 수정

```cmake
if(HSMR_USE_SDL)
    set(PLATFORM_SDL_SOURCES
        SDL/SDLPlatform.h
        SDL/Private/SDLPlatform.cpp
    )
    # Win/Mac 폴더는 빌드에서 제외
else()
    # 기존 네이티브 코드 유지 (fallback)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE
    Core
    $<$<BOOL:${HSMR_USE_SDL}>:${HS_SDL3_LIB}>
)
```

---

## 3단계: RHI - Vulkan Surface 생성

### VulkanContext.cpp 수정

**`Source/RHI/Vulkan/Private/VulkanContext.cpp`**

Surface 생성 함수 수정:
```cpp
VkSurfaceKHR VulkanContext::createSurface(const NativeWindow& nativeWindow)
{
#ifdef HSMR_USE_SDL
    SDL_Window* sdlWindow = GetSDLWindow();
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    SDL_Vulkan_CreateSurface(sdlWindow, _instanceVk, nullptr, &surface);
    return surface;
#else
    // 기존 Win32 surface 생성 코드
#endif
}
```

Instance 확장 목록 수정:
```cpp
#ifdef HSMR_USE_SDL
    uint32 count = 0;
    const char* const* sdlExts = SDL_Vulkan_GetInstanceExtensions(&count);
    for (uint32 i = 0; i < count; i++)
        extensions.push_back(sdlExts[i]);
#else
    extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
```

### RHI/CMakeLists.txt

변경 없음 - Windows 빌드에서 자동으로 Vulkan만 빌드됨

---

## 4단계: ImGui 선택적 통합

### Editor/CMakeLists.txt 수정

```cmake
if(HSMR_ENABLE_IMGUI)
    set(THIRDPARTY_IMGUI ...)

    if(HSMR_USE_SDL)
        list(APPEND THIRDPARTY_IMGUI
            ThirdParty/ImGui/imgui_impl_sdl3.h
            ThirdParty/ImGui/imgui_impl_sdl3.cpp
            ThirdParty/ImGui/imgui_impl_vulkan.h
            ThirdParty/ImGui/imgui_impl_vulkan.cpp
        )
    endif()
endif()
```

### SDL ImGui Backend 연동

**`Source/Editor/GUI/Private/ImGuiExtension.cpp`** 수정:
```cpp
#ifdef HSMR_USE_SDL
#include "ImGui/imgui_impl_sdl3.h"
// ImGui_ImplSDL3_InitForVulkan() 사용
// ImGui_ImplSDL3_NewFrame() 사용
#endif
```

SDL 이벤트 루프에서 ImGui 처리:
```cpp
// SDLPlatform.cpp의 PollNativeEventInternal()에서
#ifdef HSMR_ENABLE_IMGUI
    ImGui_ImplSDL3_ProcessEvent(&event);
#endif
```

---

## 수정해야 할 주요 파일

| 파일 | 변경 내용 |
|------|----------|
| `CMakeLists.txt` (root) | 옵션 추가, SDL3 경로 |
| `Source/Platform/CMakeLists.txt` | SDL 조건부 빌드 |
| `Source/Platform/SDL/*` | **신규 생성** |
| `Source/RHI/Vulkan/Private/VulkanContext.cpp` | SDL surface 생성 |
| `Source/Editor/CMakeLists.txt` | ImGui 선택적 빌드 |
| `Source/Editor/GUI/Private/ImGuiExtension.cpp` | SDL3 backend 사용 |
| `Source/Application/CMakeLists.txt` | SDL 링크 추가 |

---

## 검증

### 빌드 테스트
```bash
cmake -S . -B Build -DHSMR_USE_SDL=ON -DHSMR_ENABLE_IMGUI=ON
cmake --build Build --config Debug
```

### 기능 체크리스트
- [ ] 윈도우 생성 및 표시
- [ ] 키보드/마우스 입력 동작
- [ ] Vulkan surface 생성 성공
- [ ] 스왑체인 동작 및 렌더링
- [ ] ImGui 렌더링 (활성화 시)
- [ ] 윈도우 리사이즈 이벤트

### 빌드 설정 조합
| SDL | ImGui | 설명 |
|-----|-------|------|
| ON | ON | 전체 기능 (권장) |
| ON | OFF | 최소 렌더러 |
| OFF | ON | 네이티브 Win32 fallback |

---

## 참고 사항

- `NativeWindow.h:17`에 이미 `// Same with SDL_WindowFlags.` 명시됨 → 플래그 호환성 보장
- `imgui_impl_sdl3.h` 이미 프로젝트에 존재 → SDL3 사용 권장
- 기존 Platform/Win, Platform/Mac 코드는 삭제하지 않고 조건부 빌드로 유지 (fallback)

---

## 아키텍처 다이어그램

```
┌─────────────────────────────────────────────────────────┐
│                      Application                         │
├─────────────────────────────────────────────────────────┤
│                        Engine                            │
├──────────────────────┬──────────────────────────────────┤
│   Editor (ImGui)     │         Renderer                 │
│   [선택적]            │                                  │
├──────────────────────┴──────────────────────────────────┤
│                    RHI (Vulkan)                          │
├─────────────────────────────────────────────────────────┤
│                   Platform (SDL3)                        │
│            윈도우 생성, 이벤트, 입력 처리                  │
└─────────────────────────────────────────────────────────┘
```

---

# 구현 로그

## Phase 1: CMake Configuration - 완료

**날짜**: 2026-01-29

### 수행 작업

1. **Root CMakeLists.txt 수정** (`CMakeLists.txt:70-95`)
   - `HSMR_USE_SDL` 옵션 추가 (기본값: ON)
   - `HSMR_ENABLE_IMGUI` 옵션 추가 (기본값: ON)
   - SDL3 경로 변수 설정:
     - `HS_SDL3_INCLUDE_DIR`
     - `HS_SDL3_LIB`
     - `HS_SDL3_DLL`
   - `HSMR_USE_SDL`, `HSMR_ENABLE_IMGUI` 컴파일 정의 추가

2. **SDL3 의존성 파일 배치**
   ```
   Dependency/
   ├── include/SDL3/
   │   ├── SDL.h
   │   ├── SDL_vulkan.h
   │   └── ... (84개 헤더 파일)
   ├── lib/win/
   │   ├── Debug/
   │   │   ├── SDL3.lib
   │   │   └── SDL3.pdb
   │   └── Release/
   │       └── SDL3.lib
   └── dll/win/
       └── Debug/
           └── SDL3.dll
   ```

### 변경된 파일
- `CMakeLists.txt` (root)

### 다음 단계
Phase 2: Platform/SDL 구현 파일 생성

---

## Phase 2: Platform/SDL 구현 - 완료

**날짜**: 2026-01-29

### 수행 작업

1. **새 파일 생성**
   - `Source/Platform/SDL/SDLPlatform.h` - SDL 플랫폼 헤더
   - `Source/Platform/SDL/Private/SDLPlatform.cpp` - SDL 플랫폼 구현

2. **SDLPlatform.cpp 주요 구현 내용**
   - `GetSDLWindow()` - Vulkan surface 생성용 SDL 윈도우 핸들 반환
   - `MapSDLScancodeToVK()` - SDL 스캔코드 → Windows VK 코드 매핑 (Input 시스템 호환)
   - `CreateNativeWindowInternal()` - SDL_CreateWindow + NativeWindow 구조체 초기화
   - `DestroyNativeWindowInternal()` - SDL_DestroyWindow + SDL_Quit
   - `ShowNativeWindowInternal()` - SDL_ShowWindow
   - `PollNativeEventInternal()` - SDL_PollEvent + 이벤트 처리
   - `SetNativePreEventHandler()` - ImGui 이벤트 핸들러 등록

3. **Platform/CMakeLists.txt 수정**
   - `HSMR_USE_SDL` 조건부 빌드 분기 추가
   - SDL 사용 시: SDLPlatform + WinFileSystem/WinSystemContext
   - SDL 미사용 시: 기존 Win/Mac 전체 코드 (fallback)
   - SDL3 라이브러리 링킹 추가
   - SDL3.dll 복사 커맨드 추가

4. **Core/Native/Private/NativeWindow.cpp 수정**
   - `HSMR_USE_SDL` 조건부 include 추가

### 변경된 파일
- `Source/Platform/SDL/SDLPlatform.h` (신규)
- `Source/Platform/SDL/Private/SDLPlatform.cpp` (신규)
- `Source/Platform/CMakeLists.txt`
- `Source/Core/Native/Private/NativeWindow.cpp`

### SDL 이벤트 매핑

| SDL 이벤트 | NativeEvent |
|------------|-------------|
| SDL_EVENT_QUIT | WINDOW_CLOSE |
| SDL_EVENT_WINDOW_RESIZED | WINDOW_RESIZE |
| SDL_EVENT_WINDOW_MINIMIZED | WINDOW_MINIMIZE |
| SDL_EVENT_WINDOW_MAXIMIZED | WINDOW_MAXIMIZE |
| SDL_EVENT_WINDOW_RESTORED | WINDOW_RESTORE |
| SDL_EVENT_WINDOW_FOCUS_GAINED | WINDOW_FOCUS_IN |
| SDL_EVENT_WINDOW_FOCUS_LOST | WINDOW_FOCUS_OUT |
| SDL_EVENT_KEY_DOWN/UP | Input::s_button 업데이트 |
| SDL_EVENT_MOUSE_BUTTON_* | Input::s_button 업데이트 |
| SDL_EVENT_MOUSE_MOTION | Input::s_move 업데이트 |
| SDL_EVENT_MOUSE_WHEEL | Input::s_scroll 업데이트 |

### 다음 단계
Phase 3: VulkanContext SDL surface 생성

---

## Phase 3: VulkanContext SDL surface - 완료

**날짜**: 2026-01-29

### 수행 작업

1. **VulkanContext.cpp 헤더 추가** (line 15-18)
   ```cpp
   #ifdef __SDL__
   #include "Platform/SDL/SDLPlatform.h"
   #include <SDL3/SDL_vulkan.h>
   #endif
   ```

2. **createInstance() 확장 목록 수정** (line 1355-1390)
   - `__SDL__` 정의 시: `SDL_Vulkan_GetInstanceExtensions()` 사용
   - 그 외: 기존 `VK_KHR_WIN32_SURFACE_EXTENSION_NAME` 사용
   - Debug 확장은 공통으로 추가

3. **createSurface() 함수 수정** (line 1455-1478)
   - `__SDL__` 정의 시: `SDL_Vulkan_CreateSurface()` 사용
   - 그 외: 기존 `vkCreateWin32SurfaceKHR()` 사용

### 변경된 파일
- `Source/RHI/Vulkan/Private/VulkanContext.cpp`

### SDL Vulkan 통합 흐름
```
SDL_Vulkan_GetInstanceExtensions()
        ↓
    VkInstance 생성
        ↓
SDL_Vulkan_CreateSurface(sdlWindow, instance, ...)
        ↓
    VkSurfaceKHR 획득
        ↓
    Swapchain 생성
```

### 다음 단계
Phase 4: ImGui SDL3 backend 연동

---

## Phase 4: ImGui SDL3 backend - 완료

**날짜**: 2026-01-29

### 수행 작업

1. **Editor/CMakeLists.txt 수정**
   - `HSMR_USE_SDL` 조건 분기 추가
   - SDL 사용 시: `imgui_impl_sdl3.h/cpp` 포함
   - Native 사용 시: `imgui_impl_win32.h/cpp` 포함
   - SDL3 라이브러리 링킹 추가

2. **ImGuiExtensionVulkan.cpp 수정**
   - 조건부 include 추가:
     ```cpp
     #ifdef __SDL__
     #include "ImGui/imgui_impl_sdl3.h"
     #else
     #include "ImGui/imgui_impl_win32.h"
     #endif
     ```
   - `InitializeBackend()`: `ImGui_ImplSDL3_InitForVulkan()` 사용
   - `BeginRender()`: `ImGui_ImplSDL3_NewFrame()` 사용
   - `FinalizeBackend()`: `ImGui_ImplSDL3_Shutdown()` 사용
   - `SetProcessEventHandler()`: `ImGui_ImplSDL3_ProcessEvent` 반환

3. **SDLPlatform.cpp 이벤트 처리 수정**
   - ImGui 이벤트 핸들러 호출 후 윈도우 이벤트는 항상 처리

### 변경된 파일
- `Source/Editor/CMakeLists.txt`
- `Source/Editor/GUI/Vulkan/ImGuiExtensionVulkan.cpp`
- `Source/Platform/SDL/Private/SDLPlatform.cpp`

### ImGui 초기화 흐름
```
SDL_Window 생성
      ↓
ImGui_ImplSDL3_InitForVulkan(sdlWindow)
      ↓
ImGui_ImplVulkan_Init(&initInfo)
      ↓
[매 프레임]
ImGui_ImplVulkan_NewFrame()
ImGui_ImplSDL3_NewFrame()
ImGui::NewFrame()
      ↓
[종료 시]
ImGui_ImplVulkan_Shutdown()
ImGui_ImplSDL3_Shutdown()
```

---

# 구현 완료

## 전체 변경 파일 목록

| 파일 | Phase | 변경 내용 |
|------|-------|----------|
| `CMakeLists.txt` | 1 | SDL3 옵션, 경로 설정 |
| `Source/Platform/SDL/SDLPlatform.h` | 2 | 신규 생성 |
| `Source/Platform/SDL/Private/SDLPlatform.cpp` | 2,4 | 신규 생성, 이벤트 처리 수정 |
| `Source/Platform/CMakeLists.txt` | 2 | SDL 조건부 빌드 |
| `Source/Core/Native/Private/NativeWindow.cpp` | 2 | SDL 헤더 포함 |
| `Source/RHI/Vulkan/Private/VulkanContext.cpp` | 3 | SDL surface 생성 |
| `Source/Editor/CMakeLists.txt` | 4 | SDL3 backend 포함 |
| `Source/Editor/GUI/Vulkan/ImGuiExtensionVulkan.cpp` | 4 | SDL3 backend 사용 |

## 빌드 명령

```bash
# CMake 설정
cmake -S . -B Build -DHSMR_USE_SDL=ON -DHSMR_ENABLE_IMGUI=ON

# 빌드
cmake --build Build --config Debug
```

## 버그 수정

### LNK2019: GetSDLWindow 미해결 외부 심볼 오류 - 해결

**문제**: Editor.dll 빌드 시 `GetSDLWindow()` 링커 오류 발생

```
LNK2019: unresolved external symbol "struct SDL_Window * __cdecl hs::GetSDLWindow(void)"
```

**원인**:
- Editor → Engine → Platform 의존성 구조에서 Platform은 정적 라이브러리
- `GetSDLWindow()`가 Platform에 정의되어 있지만 Editor에서 직접 호출 불가
- Platform을 Editor에 추가 링크하면 중복 심볼(LNK2005) 오류 발생

**해결**: `nativeWindow->handle`에 이미 SDL_Window*가 저장되어 있으므로 직접 사용

1. **VulkanContext.cpp** (line 1462)
   ```cpp
   // 변경 전
   SDL_Window* sdlWindow = hs::GetSDLWindow();

   // 변경 후
   SDL_Window* sdlWindow = static_cast<SDL_Window*>(nativeWindow.handle);
   ```

2. **ImGuiExtensionVulkan.cpp** (line 92)
   ```cpp
   // 변경 전
   #include "Platform/SDL/SDLPlatform.h"
   SDL_Window* sdlWindow = hs::GetSDLWindow();

   // 변경 후
   #include <SDL3/SDL.h>
   SDL_Window* sdlWindow = static_cast<SDL_Window*>(nativeWindow->handle);
   ```

**결과**: 빌드 성공 ✅

---

## 검증 체크리스트
- [x] CMake 설정 성공
- [x] 빌드 성공
- [ ] 윈도우 생성 및 표시
- [ ] Vulkan surface 생성
- [ ] 스왑체인 동작
- [ ] ImGui 렌더링
- [ ] 키보드/마우스 입력
