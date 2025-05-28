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
	WNDCLASS wc = {};
	wc.lpfnWndProc = wnd_proc;
	wc.lpszClassName = name;

	RegisterClass(&wc);

	int wNameLen = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
	wchar_t* wName = new wchar_t[wNameLen];
	MultiByteToWideChar(CP_UTF8, 0, name, -1, wName, wNameLen);

	HINSTANCE hInstance = (HINSTANCE)hs_platform_get_hinstance();
	HWND hWnd = CreateWindowW(
		L"HSMR",
		L"HSMR",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		(int)width, (int)height,
		NULL,
		NULL,
		hInstance,
		NULL);

	if (hWnd == nullptr)
	{
		auto err = GetLastError();
		HS_LOG(crash, "Cannot create native window. Error code: %d", err);
	}

	outNativeWindow = {};
	outNativeWindow.width = width;
	outNativeWindow.height = height;
	outNativeWindow.flags = flag;
	outNativeWindow.title = name;
	outNativeWindow.minWidth = 1;
	outNativeWindow.minHeight = 1;
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

void hs_platform_window_poll_event()
{

}

void hs_platform_window_set_position(NativeWindow& nativeWindow, uint16 x, uint16 y)
{

}

void hs_platform_window_get_position(NativeWindow& nativeWindow)
{

}

HS_NS_END