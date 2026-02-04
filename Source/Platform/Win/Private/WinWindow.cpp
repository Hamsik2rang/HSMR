#include "Platform/Win/WinWindow.h"

#include <cassert>

#include "Core/HAL/Input.h"
#include "Core/Log.h"

#ifdef __SDL__
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// SDL-specific static variables
static SDL_Window* s_sdlWindow = nullptr;
static hs::NativeWindow* s_boundHsWindow = nullptr;
static bool s_sdlInitialized = false;

// Pre-event handler for ImGui integration
static bool (*s_preEventHandler)(SDL_Event*) = nullptr;

namespace {
// Map SDL scancode to Input::Button enum value
static uint8 MapSDLScancodeToButton(SDL_Scancode scancode)
{
    using Button = hs::Input::Button;

    switch (scancode)
    {
        // Letters A-Z
        case SDL_SCANCODE_A: return static_cast<uint8>(Button::A);
        case SDL_SCANCODE_B: return static_cast<uint8>(Button::B);
        case SDL_SCANCODE_C: return static_cast<uint8>(Button::C);
        case SDL_SCANCODE_D: return static_cast<uint8>(Button::D);
        case SDL_SCANCODE_E: return static_cast<uint8>(Button::E);
        case SDL_SCANCODE_F: return static_cast<uint8>(Button::F);
        case SDL_SCANCODE_G: return static_cast<uint8>(Button::G);
        case SDL_SCANCODE_H: return static_cast<uint8>(Button::H);
        case SDL_SCANCODE_I: return static_cast<uint8>(Button::I);
        case SDL_SCANCODE_J: return static_cast<uint8>(Button::J);
        case SDL_SCANCODE_K: return static_cast<uint8>(Button::K);
        case SDL_SCANCODE_L: return static_cast<uint8>(Button::L);
        case SDL_SCANCODE_M: return static_cast<uint8>(Button::M);
        case SDL_SCANCODE_N: return static_cast<uint8>(Button::N);
        case SDL_SCANCODE_O: return static_cast<uint8>(Button::O);
        case SDL_SCANCODE_P: return static_cast<uint8>(Button::P);
        case SDL_SCANCODE_Q: return static_cast<uint8>(Button::Q);
        case SDL_SCANCODE_R: return static_cast<uint8>(Button::R);
        case SDL_SCANCODE_S: return static_cast<uint8>(Button::S);
        case SDL_SCANCODE_T: return static_cast<uint8>(Button::T);
        case SDL_SCANCODE_U: return static_cast<uint8>(Button::U);
        case SDL_SCANCODE_V: return static_cast<uint8>(Button::V);
        case SDL_SCANCODE_W: return static_cast<uint8>(Button::W);
        case SDL_SCANCODE_X: return static_cast<uint8>(Button::X);
        case SDL_SCANCODE_Y: return static_cast<uint8>(Button::Y);
        case SDL_SCANCODE_Z: return static_cast<uint8>(Button::Z);

        // Numbers 0-9
        case SDL_SCANCODE_0: return static_cast<uint8>(Button::NUM_0);
        case SDL_SCANCODE_1: return static_cast<uint8>(Button::NUM_1);
        case SDL_SCANCODE_2: return static_cast<uint8>(Button::NUM_2);
        case SDL_SCANCODE_3: return static_cast<uint8>(Button::NUM_3);
        case SDL_SCANCODE_4: return static_cast<uint8>(Button::NUM_4);
        case SDL_SCANCODE_5: return static_cast<uint8>(Button::NUM_5);
        case SDL_SCANCODE_6: return static_cast<uint8>(Button::NUM_6);
        case SDL_SCANCODE_7: return static_cast<uint8>(Button::NUM_7);
        case SDL_SCANCODE_8: return static_cast<uint8>(Button::NUM_8);
        case SDL_SCANCODE_9: return static_cast<uint8>(Button::NUM_9);

        // Function keys
        case SDL_SCANCODE_F1:  return static_cast<uint8>(Button::F1);
        case SDL_SCANCODE_F2:  return static_cast<uint8>(Button::F2);
        case SDL_SCANCODE_F3:  return static_cast<uint8>(Button::F3);
        case SDL_SCANCODE_F4:  return static_cast<uint8>(Button::F4);
        case SDL_SCANCODE_F5:  return static_cast<uint8>(Button::F5);
        case SDL_SCANCODE_F6:  return static_cast<uint8>(Button::F6);
        case SDL_SCANCODE_F7:  return static_cast<uint8>(Button::F7);
        case SDL_SCANCODE_F8:  return static_cast<uint8>(Button::F8);
        case SDL_SCANCODE_F9:  return static_cast<uint8>(Button::F9);
        case SDL_SCANCODE_F10: return static_cast<uint8>(Button::F10);
        case SDL_SCANCODE_F11: return static_cast<uint8>(Button::F11);
        case SDL_SCANCODE_F12: return static_cast<uint8>(Button::F12);

        // Special keys
        case SDL_SCANCODE_BACKSPACE: return static_cast<uint8>(Button::BACK);
        case SDL_SCANCODE_TAB:       return static_cast<uint8>(Button::TAB);
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:    return static_cast<uint8>(Button::SHIFT);
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:     return static_cast<uint8>(Button::CONTROL);
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_RALT:      return static_cast<uint8>(Button::ALT);
        case SDL_SCANCODE_SPACE:     return static_cast<uint8>(Button::SPACE);
        case SDL_SCANCODE_END:       return static_cast<uint8>(Button::END);
        case SDL_SCANCODE_HOME:      return static_cast<uint8>(Button::HOME);
        case SDL_SCANCODE_LEFT:      return static_cast<uint8>(Button::LEFT);
        case SDL_SCANCODE_UP:        return static_cast<uint8>(Button::UP);
        case SDL_SCANCODE_RIGHT:     return static_cast<uint8>(Button::RIGHT);
        case SDL_SCANCODE_DOWN:      return static_cast<uint8>(Button::DOWN);
        case SDL_SCANCODE_INSERT:    return static_cast<uint8>(Button::INSERT);
        case SDL_SCANCODE_DELETE:    return static_cast<uint8>(Button::DELETE);

        // Numpad
        case SDL_SCANCODE_KP_0: return static_cast<uint8>(Button::NUMPAD_0);
        case SDL_SCANCODE_KP_1: return static_cast<uint8>(Button::NUMPAD_1);
        case SDL_SCANCODE_KP_2: return static_cast<uint8>(Button::NUMPAD_2);
        case SDL_SCANCODE_KP_3: return static_cast<uint8>(Button::NUMPAD_3);
        case SDL_SCANCODE_KP_4: return static_cast<uint8>(Button::NUMPAD_4);
        case SDL_SCANCODE_KP_5: return static_cast<uint8>(Button::NUMPAD_5);
        case SDL_SCANCODE_KP_6: return static_cast<uint8>(Button::NUMPAD_6);
        case SDL_SCANCODE_KP_7: return static_cast<uint8>(Button::NUMPAD_7);
        case SDL_SCANCODE_KP_8: return static_cast<uint8>(Button::NUMPAD_8);
        case SDL_SCANCODE_KP_9: return static_cast<uint8>(Button::NUMPAD_9);
        case SDL_SCANCODE_KP_MULTIPLY: return static_cast<uint8>(Button::MULTIPLY);
        case SDL_SCANCODE_KP_PLUS:     return static_cast<uint8>(Button::ADD);
        case SDL_SCANCODE_KP_MINUS:    return static_cast<uint8>(Button::SUBTRACT);
        case SDL_SCANCODE_KP_PERIOD:   return static_cast<uint8>(Button::DECIMAL);
        case SDL_SCANCODE_KP_DIVIDE:   return static_cast<uint8>(Button::DIVIDE);

        // Windows/Command key
        case SDL_SCANCODE_LGUI: return static_cast<uint8>(Button::LWIN_OR_COMMAND);
        case SDL_SCANCODE_RGUI: return static_cast<uint8>(Button::RWIN);

        default: return static_cast<uint8>(Button::UNKNOWN);
    }
}
} // anonymous namespace

#else // !__SDL__ - Native Win32 path

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace hs;

static hs::NativeWindow* s_boundHsWindow                 = nullptr;
LRESULT (*s_preEventHandler)(HWND, UINT, WPARAM, LPARAM) = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (s_preEventHandler && s_preEventHandler(hWnd, msg, wParam, lParam))
    {
        return true;
    }

    if (nullptr == s_boundHsWindow)
    {
        return true;
    }

    switch (msg)
    {
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;

        break;
    }
    case WM_CREATE:
    {
        PushNativeEvent(s_boundHsWindow, hs::NativeEvent::Type::WINDOW_OPEN);

        break;
    }
    case WM_ACTIVATE:
    {
        // @NOTICE: WM_ACTIVATE는 활성화되어 있는 동안 계속 토글됨
        break;
    }
    case WM_COMMAND:
    {
    }
    break;
    case WM_SIZING:
    {
        break;
    }
    case WM_SIZE:
    {
        const int width        = LOWORD(lParam);
        const int height       = HIWORD(lParam);
        const bool isMinimized = (wParam == SIZE_MINIMIZED);
        const bool isMaximized = wParam == SIZE_MAXIMIZED || (s_boundHsWindow->isMaximized && wParam != SIZE_RESTORED);

        hs::NativeEvent event;
        if (isMinimized)
        {
            PushNativeEvent(s_boundHsWindow, hs::NativeEvent::Type::WINDOW_MINIMIZE);
        }
        else if (isMaximized)
        {
            PushNativeEvent(s_boundHsWindow, hs::NativeEvent::Type::WINDOW_MAXIMIZE);
        }
        else
        {
            if (width != s_boundHsWindow->surfaceWidth || height != s_boundHsWindow->surfaceHeight)
            {
                PushNativeEvent(s_boundHsWindow, hs::NativeEvent{hs::NativeEvent::Type::WINDOW_RESIZE});
            }
        }

        RECT rect;
        GetClientRect(hWnd, &rect);
        s_boundHsWindow->surfaceWidth  = rect.right - rect.left;
        s_boundHsWindow->surfaceHeight = rect.bottom - rect.top;

        GetWindowRect(hWnd, &rect);
        s_boundHsWindow->width  = rect.right - rect.left;
        s_boundHsWindow->height = rect.bottom - rect.top;

        break;
    }
    case WM_ENTERSIZEMOVE:
    {
        PushNativeEvent(s_boundHsWindow, hs::NativeEvent::Type::WINDOW_MOVE_ENTER);

        break;
    }
    case WM_EXITSIZEMOVE:
    {
        PushNativeEvent(s_boundHsWindow, hs::NativeEvent::Type::WINDOW_MOVE_EXIT);
        break;
    }
    case WM_MOVE:
    {
        break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        uint8 keyCode = static_cast<uint8>(wParam);
        if (keyCode < static_cast<uint8>(Input::Button::COUNT))
        {
            Input::s_button[keyCode].isPressed = 1;
            Input::s_button[keyCode].repeatCount = (lParam & 0xFFFF);
        }
        break;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        uint8 keyCode = static_cast<uint8>(wParam);
        if (keyCode < static_cast<uint8>(Input::Button::COUNT))
        {
            Input::s_button[keyCode].isPressed = 0;
            Input::s_button[keyCode].repeatCount = 0;
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_LEFT)].isPressed = 1;
        break;
    }
    case WM_LBUTTONUP:
    {
        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_LEFT)].isPressed = 0;
        break;
    }
    case WM_RBUTTONDOWN:
    {
        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_RIGHT)].isPressed = 1;
        break;
    }
    case WM_RBUTTONUP:
    {
        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_RIGHT)].isPressed = 0;
        break;
    }
    case WM_MBUTTONDOWN:
    {
        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_MIDDLE)].isPressed = 1;
        break;
    }
    case WM_MBUTTONUP:
    {
        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_MIDDLE)].isPressed = 0;
        break;
    }
    case WM_MOUSEMOVE:
    {
        Input::s_move.xPoint = LOWORD(lParam);
        Input::s_move.yPoint = HIWORD(lParam);
        Input::s_move.isMoved = 1;
        break;
    }
    case WM_MOUSEWHEEL:
    {
        Input::s_scroll.vOffset = GET_WHEEL_DELTA_WPARAM(wParam);
        Input::s_scroll.isScrolled = 1;
        break;
    }
    case WM_MOUSEHWHEEL:
    {
        Input::s_scroll.hOffset = GET_WHEEL_DELTA_WPARAM(wParam);
        Input::s_scroll.isScrolled = 1;
        break;
    }
    case WM_DESTROY:
    case WM_CLOSE:
    {
        PushNativeEvent(s_boundHsWindow, hs::NativeEvent::Type::WINDOW_CLOSE);
        PostQuitMessage(0);
        break;
    }
    default:
        break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

#endif // !__SDL__

HS_NS_BEGIN

bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow)
{
#ifdef __SDL__
    // === SDL3 + Vulkan path ===

    // Convert EWindowFlags to SDL_WindowFlags and ensure Vulkan flag is set
    SDL_WindowFlags sdlFlags = static_cast<SDL_WindowFlags>(static_cast<uint64>(flag));
    sdlFlags |= SDL_WINDOW_VULKAN;

    // Create SDL window
    s_sdlWindow = SDL_CreateWindow(name, width, height, sdlFlags);
    if (!s_sdlWindow)
    {
        HS_LOG(crash, "Failed to create SDL window: %s", SDL_GetError());
        return false;
    }

    s_boundHsWindow = &outNativeWindow;

    // Get actual drawable size (may differ on high-DPI displays)
    int drawableWidth, drawableHeight;
    SDL_GetWindowSizeInPixels(s_sdlWindow, &drawableWidth, &drawableHeight);

    float displayScale = SDL_GetWindowDisplayScale(s_sdlWindow);

    // Populate NativeWindow struct
    outNativeWindow = {};
    outNativeWindow.handle = s_sdlWindow;
    outNativeWindow.graphicsView = nullptr;   // Vulkan doesn't use these
    outNativeWindow.graphicsLayer = nullptr;
    outNativeWindow.width = width;
    outNativeWindow.height = height;
    outNativeWindow.surfaceWidth = static_cast<uint16>(drawableWidth);
    outNativeWindow.surfaceHeight = static_cast<uint16>(drawableHeight);
    outNativeWindow.flags = flag;
    outNativeWindow.title = name;
    outNativeWindow.scale = displayScale;
    outNativeWindow.isMaximized = (static_cast<uint64>(flag) & SDL_WINDOW_MAXIMIZED) != 0;
    outNativeWindow.isMinimized = (static_cast<uint64>(flag) & SDL_WINDOW_MINIMIZED) != 0;
    outNativeWindow.resizable = (flag & EWindowFlags::WINDOW_RESIZABLE) != EWindowFlags::NONE;
    outNativeWindow.useHDR = (flag & EWindowFlags::WINDOW_HIGH_PIXEL_DENSITY) != EWindowFlags::NONE;

    HS_LOG(info, "[SDL] Window created: %s (%dx%d, surface: %dx%d, scale: %.2f)",
           name, width, height, drawableWidth, drawableHeight, displayScale);

#else
    // === Native Win32 API path ===

    s_boundHsWindow = &outNativeWindow;

    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    WNDCLASSEX wcex    = {};
    wcex.cbSize        = sizeof(WNDCLASSEXW);
    wcex.style         = CS_CLASSDC;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.lpfnWndProc   = WndProc;
    wcex.lpszClassName = "HSMR";
    wcex.hInstance     = hInstance;
    wcex.lpfnWndProc   = WndProc;
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE("HSMR"));
    wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName  = "HSMR";
    wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE("HSMR ICON"));

    auto result = RegisterClassEx(&wcex);

    if (!result)
    {
        return false;
    }

    int wNameLen   = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
    wchar_t* wName = new wchar_t[wNameLen];
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wName, wNameLen);

    UINT dpi  = GetDpiForSystem(); // DPI 조회
    RECT rect = {0, 0, width, height};
    AdjustWindowRectExForDpi(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0, dpi);

    HWND hWnd = CreateWindowEx(
        0,                   // Optional window styles.
        wcex.lpszClassName,  // Window class
        name,                // Window text
        WS_OVERLAPPEDWINDOW, // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, (int)(rect.right - rect.left), (int)(rect.bottom - rect.top),

        NULL,
        NULL,
        hInstance,
        NULL // Additional application data
    );

    if (hWnd == nullptr)
    {
        auto err = GetLastError();
        return false;
    }

    RECT surfaceRect;
    GetClientRect(hWnd, &surfaceRect);

    RECT windowRect;
    GetWindowRect(hWnd, &windowRect);

    // static_assert(windowRect.right - windowRect.left == width, "Window width is not same with surface width.");
    // static_assert(windowRect.bottom - windowRect.top == height, "Window height is not same with surface height.");
    if (!(surfaceRect.right - surfaceRect.left == width && surfaceRect.bottom - surfaceRect.top == height))
    {
        assert(false);
    }
    outNativeWindow               = {};
    outNativeWindow.width         = width;
    outNativeWindow.height        = height;
    outNativeWindow.surfaceWidth  = surfaceRect.right - surfaceRect.left;
    outNativeWindow.surfaceHeight = surfaceRect.bottom - surfaceRect.top;
    outNativeWindow.flags         = flag;
    outNativeWindow.title         = name;
    outNativeWindow.scale         = (float)dpi / (float)USER_DEFAULT_SCREEN_DPI;
    outNativeWindow.handle        = hWnd;
    outNativeWindow.graphicsView  = nullptr;  // Vulkan doesn't use these
    outNativeWindow.graphicsLayer = nullptr;
    outNativeWindow.isMaximized   = false;
    outNativeWindow.isMinimized   = false;
    outNativeWindow.resizable     = (flag & EWindowFlags::WINDOW_RESIZABLE) != EWindowFlags::NONE;
    outNativeWindow.useHDR        = (flag & EWindowFlags::WINDOW_HIGH_PIXEL_DENSITY) != EWindowFlags::NONE;

    // TODO: DISPLAY_DEVICEW, DEVMODEW 사용해서 추가 caps가져오기.
#endif

    return true;
}

void DestroyNativeWindowInternal(NativeWindow& nativeWindow)
{
#ifdef __SDL__
    if (s_sdlWindow)
    {
        SDL_DestroyWindow(s_sdlWindow);
        s_sdlWindow = nullptr;
    }

    if (s_sdlInitialized)
    {
        SDL_Quit();
        s_sdlInitialized = false;
    }

    s_boundHsWindow = nullptr;
    nativeWindow.graphicsView = nullptr;
    nativeWindow.graphicsLayer = nullptr;
#else
    // empty
#endif
}

void ShowNativeWindowInternal(const NativeWindow& nativeWindow)
{
#ifdef __SDL__
    if (s_sdlWindow)
    {
        SDL_ShowWindow(s_sdlWindow);
    }
#else
    ShowWindow((HWND)nativeWindow.handle, SW_SHOW);
#endif
}

void PollNativeEventInternal(NativeWindow& nativeWindow)
{
    // Reset per-frame input states
    Input::s_move.isMoved = 0;
    Input::s_scroll.isScrolled = 0;

#ifdef __SDL__
    s_boundHsWindow = &nativeWindow;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // Let ImGui process the event first
        if (s_preEventHandler)
        {
            s_preEventHandler(&event);
        }

        // Process window and input events
        switch (event.type)
        {
            // Window events
            case SDL_EVENT_QUIT:
                PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_CLOSE);
                break;

            case SDL_EVENT_WINDOW_RESIZED:
            {
                int w, h;
                SDL_GetWindowSizeInPixels(s_sdlWindow, &w, &h);
                nativeWindow.surfaceWidth = static_cast<uint16>(w);
                nativeWindow.surfaceHeight = static_cast<uint16>(h);

                SDL_GetWindowSize(s_sdlWindow, &w, &h);
                nativeWindow.width = static_cast<uint16>(w);
                nativeWindow.height = static_cast<uint16>(h);

                PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_RESIZE);
                break;
            }

            case SDL_EVENT_WINDOW_MINIMIZED:
                nativeWindow.isMinimized = true;
                nativeWindow.isMaximized = false;
                PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_MINIMIZE);
                break;

            case SDL_EVENT_WINDOW_MAXIMIZED:
                nativeWindow.isMaximized = true;
                nativeWindow.isMinimized = false;
                PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_MAXIMIZE);
                break;

            case SDL_EVENT_WINDOW_RESTORED:
                nativeWindow.isMinimized = false;
                nativeWindow.isMaximized = false;
                PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_RESTORE);
                break;

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_FOCUS_IN);
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_FOCUS_OUT);
                break;

            // Keyboard events
            case SDL_EVENT_KEY_DOWN:
            {
                uint8 keyCode = MapSDLScancodeToButton(event.key.scancode);
                if (keyCode > 0 && keyCode < static_cast<uint8>(Input::Button::COUNT))
                {
                    Input::s_button[keyCode].isPressed = 1;
                    Input::s_button[keyCode].repeatCount = event.key.repeat ? 1 : 0;
                }
                break;
            }

            case SDL_EVENT_KEY_UP:
            {
                uint8 keyCode = MapSDLScancodeToButton(event.key.scancode);
                if (keyCode > 0 && keyCode < static_cast<uint8>(Input::Button::COUNT))
                {
                    Input::s_button[keyCode].isPressed = 0;
                    Input::s_button[keyCode].repeatCount = 0;
                }
                break;
            }

            // Mouse button events
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                switch (event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_LEFT)].isPressed = 1;
                        break;
                    case SDL_BUTTON_RIGHT:
                        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_RIGHT)].isPressed = 1;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_MIDDLE)].isPressed = 1;
                        break;
                }
                break;
            }

            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                switch (event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_LEFT)].isPressed = 0;
                        break;
                    case SDL_BUTTON_RIGHT:
                        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_RIGHT)].isPressed = 0;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        Input::s_button[static_cast<uint8>(Input::Button::MOUSE_MIDDLE)].isPressed = 0;
                        break;
                }
                break;
            }

            // Mouse motion
            case SDL_EVENT_MOUSE_MOTION:
                Input::s_move.xPoint = static_cast<uint16>(event.motion.x);
                Input::s_move.yPoint = static_cast<uint16>(event.motion.y);
                Input::s_move.isMoved = 1;
                break;

            // Mouse wheel
            case SDL_EVENT_MOUSE_WHEEL:
                // SDL wheel values are smaller, scale to match Windows WHEEL_DELTA (120)
                Input::s_scroll.vOffset = static_cast<int16>(event.wheel.y * 120.0f);
                Input::s_scroll.hOffset = static_cast<int16>(event.wheel.x * 120.0f);
                Input::s_scroll.isScrolled = 1;
                break;
        }
    }
#else
    s_boundHsWindow = &nativeWindow;
    MSG msg;
    while (::PeekMessage(&msg, (HWND)nativeWindow.handle, 0u, 0u, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
#endif
}

void SetNativeWindowPosition(NativeWindow& nativeWindow, uint16 x, uint16 y)
{
#ifdef __SDL__
    if (s_sdlWindow)
    {
        SDL_SetWindowPosition(s_sdlWindow, x, y);
    }
#else
    // Native implementation (empty)
#endif
}

void GetNativeWindowPosition(NativeWindow& nativeWindow)
{
#ifdef __SDL__
    if (s_sdlWindow)
    {
        int x, y;
        SDL_GetWindowPosition(s_sdlWindow, &x, &y);
        // Store position if needed
    }
#else
    // Native implementation (empty)
#endif
}

void SetNativeWindowSizeInternal(uint16 width, uint16 height)
{
#ifdef __SDL__
    if (s_sdlWindow)
    {
        SDL_SetWindowSize(s_sdlWindow, width, height);
    }
#else
    // Native implementation (empty)
#endif
}

void GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight)
{
#ifdef __SDL__
    if (s_sdlWindow)
    {
        int w, h;
        SDL_GetWindowSize(s_sdlWindow, &w, &h);
        outWidth = static_cast<uint16>(w);
        outHeight = static_cast<uint16>(h);
    }
#else
    // Native implementation (empty)
#endif
}

#pragma region Platform-dependent functions
void SetNativePreEventHandler(void* fnHandler)
{
#ifdef __SDL__
    s_preEventHandler = reinterpret_cast<bool(*)(SDL_Event*)>(fnHandler);
#else
    LRESULT (*func)(HWND, UINT, WPARAM, LPARAM) = (LRESULT (*)(HWND, UINT, WPARAM, LPARAM))fnHandler;
    s_preEventHandler                           = func;
    if (s_preEventHandler == nullptr)
    {
        HS_DEBUG_BREAK();
    }
#endif
}
#pragma endregion

HS_NS_END
