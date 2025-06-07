#include "Engine/Platform/Windows/PlatformWindowWindows.h"

#include "Engine/Platform/Windows/PlatformApplicationWindows.h"

#include "Engine/Core/Window.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

LRESULT CALLBACK wnd_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
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
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

HS_NS_BEGIN

bool hs_platform_window_create(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow)
{
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
	outNativeWindow.shouldRender = true;
	outNativeWindow.shuoldUpdate = true;
	outNativeWindow.shouldClose = false;

	// TODO: DISPLAY_DEVICEW, DEVMODEW 사용해서 추가 caps가져오기.


	return true;
}
void hs_platform_window_destroy(NativeWindow& nativeWindow)
{

}

void hs_platform_window_show(const NativeWindow& nativeWindow)
{
	ShowWindow(static_cast<HWND>(nativeWindow.handle), SW_SHOW);
}

void hs_platform_window_poll_event(const NativeWindow& nativeWindow)
{
	MSG msg;
	if (::PeekMessage(&msg, (HWND)nativeWindow.handle, 0u, 0u, PM_REMOVE))
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

HS_NS_END