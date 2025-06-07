#include "Engine/Platform/Windows/PlatformWindowWindows.h"

#include "Engine/Platform/Windows/PlatformApplicationWindows.h"

#include "Engine/Core/Window.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static HS::NativeWindow* s_boundHsWindow = nullptr;
std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> s_preEventHandler = nullptr;

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

		break;
	}
	case WM_ACTIVATE:
	{
		hs_window_push_event(s_boundHsWindow, HS::EWindowEvent::OPEN);
		s_boundHsWindow->shouldRender = true;
		break;
	}
	case WM_COMMAND:
	{

	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SIZING:
	{
		break;
	}
	case WM_SIZE:
	{
		const int width = LOWORD(lParam);
		const int height = HIWORD(lParam);
		const bool isMinimized = (wParam == SIZE_MINIMIZED);
		const bool isMaximized = wParam == SIZE_MAXIMIZED || (s_boundHsWindow->isMaximized && wParam != SIZE_RESTORED);

		HS::EWindowEvent event;
		if (isMinimized)
		{
			hs_window_push_event(s_boundHsWindow, HS::EWindowEvent::MINIMIZE);
		}
		else if (isMaximized)
		{
			hs_window_push_event(s_boundHsWindow, HS::EWindowEvent::MAXIMIZE);
		}
		else
		{
			if (width != s_boundHsWindow->surfaceWidth || height != s_boundHsWindow->surfaceHeight)
			{
				hs_window_push_event(s_boundHsWindow, HS::EWindowEvent::RESTORE);
			}
		}

		RECT rect;
		GetClientRect(hWnd, &rect);
		s_boundHsWindow->surfaceWidth = rect.right - rect.left;
		s_boundHsWindow->surfaceHeight = rect.bottom - rect.top;
		
		GetWindowRect(hWnd, &rect);
		s_boundHsWindow->width = rect.right - rect.left;
		s_boundHsWindow->height = rect.bottom - rect.top;

		break;
	}
	case WM_ENTERSIZEMOVE:
	{
		hs_window_push_event(s_boundHsWindow, HS::EWindowEvent::RESIZE_ENTER);

		break;
	}
	case WM_EXITSIZEMOVE:
	{
		break;
	}
	case WM_MOVE:
	{
		break;
	}
	case WM_DESTROY:
	case WM_CLOSE:
	{
		hs_window_push_event(s_boundHsWindow, HS::EWindowEvent::CLOSE);
		PostQuitMessage(0);
		break;
	}
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

HS_NS_BEGIN

bool hs_platform_window_create(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow)
{
	s_boundHsWindow = &outNativeWindow;

	HINSTANCE hInstance = (HINSTANCE)hs_platform_get_hinstance();

	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_CLASSDC;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.lpfnWndProc = wnd_proc;
	wcex.lpszClassName = "HSMR";
	wcex.hInstance = hInstance;
	wcex.lpfnWndProc = wnd_proc;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE("HSMR"));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = "HSMR";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE("HSMR ICON"));

	auto result = RegisterClassEx(&wcex);
	HS_LOG(debug, "RegisterClassEx result: %d", result);
	if (!result)
	{
		HS_LOG(crash, "Cannt Regist Window class!");
	}

	int wNameLen = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
	wchar_t* wName = new wchar_t[wNameLen];
	MultiByteToWideChar(CP_UTF8, 0, name, -1, wName, wNameLen);

	HWND hWnd = CreateWindowEx(
		0,                              // Optional window styles.
		wcex.lpszClassName,				// Window class
		name,							// Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, (int)width, (int)height,

		NULL,
		NULL,
		hInstance,
		NULL        // Additional application data
	);

	if (hWnd == nullptr)
	{
		auto err = GetLastError();
		HS_LOG(crash, "Cannot create native window. Error code: %d", err);
	}

	RECT surfaceRect;
	GetClientRect(hWnd, &surfaceRect);

	//SetWindowPos(hWnd, HWND_TOP, 0, 0, width, height, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER);

	surfaceRect;
	GetClientRect(hWnd, &surfaceRect);
	RECT windowRect;
	GetWindowRect(hWnd, &windowRect);

	HS_ASSERT(windowRect.right - windowRect.left == width, "Window width is not same with surface width.");
	HS_ASSERT(windowRect.bottom - windowRect.top == height, "Window height is not same with surface height.");

	outNativeWindow = {};
	outNativeWindow.width = width;
	outNativeWindow.height = height;
	outNativeWindow.surfaceWidth = surfaceRect.right - surfaceRect.left;
	outNativeWindow.surfaceHeight = surfaceRect.bottom - surfaceRect.top;
	outNativeWindow.flags = flag;
	outNativeWindow.title = name;
	outNativeWindow.handle = static_cast<void*>(hWnd);
	outNativeWindow.isMaximized = false;
	outNativeWindow.isMinimized = false;
	outNativeWindow.resizable = (flag & EWindowFlags::WINDOW_RESIZABLE) != EWindowFlags::NONE;
	outNativeWindow.useHDR = (flag & EWindowFlags::WINDOW_HIGH_PIXEL_DENSITY) != EWindowFlags::NONE;
	outNativeWindow.shouldRender = false;
	outNativeWindow.shuoldUpdate = true;
	outNativeWindow.shouldClose = false;

	// TODO: DISPLAY_DEVICEW, DEVMODEW 사용해서 추가 caps가져오기.

	return true;
}
void hs_platform_window_destroy(NativeWindow& nativeWindow)
{
	// empty
}

void hs_platform_window_show(const NativeWindow& nativeWindow)
{
	ShowWindow(static_cast<HWND>(nativeWindow.handle), SW_SHOW);
}

void hs_platform_window_poll_event(NativeWindow& nativeWindow)
{
	s_boundHsWindow = &nativeWindow;
	MSG msg;
	while (::PeekMessage(&msg, (HWND)nativeWindow.handle, 0u, 0u, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

void hs_platform_window_set_position(NativeWindow& nativeWindow, uint16 x, uint16 y)
{

}

void hs_platform_window_get_position(NativeWindow& nativeWindow)
{

}

void hs_platform_window_set_pre_event_handler(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> fnHandler)
{
	s_preEventHandler = fnHandler;
}

void hs_platform_window_set_size(uint16 width, uint16 height)
{

}

void hs_platform_window_get_size(uint16& outWidth, uint16& outHeight)
{

}

HS_NS_END