#include "Platform/Win/WinWindow.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cassert>

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
    {
        uint32 key = wParam;

        break;
    }
    case WM_KEYUP:
    {

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

HS_NS_BEGIN

bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow)
{
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
    outNativeWindow.scale         = dpi / USER_DEFAULT_SCREEN_DPI;
    outNativeWindow.handle        = hWnd;
    outNativeWindow.isMaximized   = false;
    outNativeWindow.isMinimized   = false;
    outNativeWindow.resizable     = (flag & EWindowFlags::WINDOW_RESIZABLE) != EWindowFlags::NONE;
    outNativeWindow.useHDR        = (flag & EWindowFlags::WINDOW_HIGH_PIXEL_DENSITY) != EWindowFlags::NONE;

    // TODO: DISPLAY_DEVICEW, DEVMODEW 사용해서 추가 caps가져오기.

    return true;
}
void DestroyNativeWindowInternal(NativeWindow& nativeWindow)
{
    // empty
}

void ShowNativeWindowInternal(const NativeWindow& nativeWindow)
{
    ShowWindow((HWND)nativeWindow.handle, SW_SHOW);
}

void PollNativeEventInternal(NativeWindow& nativeWindow)
{
    s_boundHsWindow = &nativeWindow;
    MSG msg;
    while (::PeekMessage(&msg, (HWND)nativeWindow.handle, 0u, 0u, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

void SetNativeWindowPosition(NativeWindow& nativeWindow, uint16 x, uint16 y)
{
}

void GetNativeWindowPosition(NativeWindow& nativeWindow)
{
}

void SetNativeWindowSizeInternal(uint16 width, uint16 height)
{
}

void GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight)
{
}

#pragma region Platform-dependent functions
void SetNativePreEventHandler(void* fnHandler)
{
    LRESULT (*func)(HWND, UINT, WPARAM, LPARAM) = (LRESULT (*)(HWND, UINT, WPARAM, LPARAM))fnHandler;
    s_preEventHandler                           = func;
    if (s_preEventHandler == nullptr)
    {
        HS_DEBUG_BREAK();
    }
}
#pragma endregion

HS_NS_END