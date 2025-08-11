//
//  BuiltInMaterialLayout.h
//  Engine
//
//  Created by Yongsik Im on 5/30/25.
//
#ifndef __HS_PLATFORM_WINDOW_WINDOWS_H__
#define __HS_PLATFORM_WINDOW_WINDOWS_H__

#include "Precompile.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <functional>
#ifndef HS_NATIVE_HANDLE
#define HS_NATIVE_HANDLE	\
struct Handle				\
{							\
	HWND hWnd;				\
	HINSTANCE hInstance;	\
}
#endif

HS_NS_BEGIN

void SetNativePreEventHandler(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> fnHandler);

HS_NS_END

#endif