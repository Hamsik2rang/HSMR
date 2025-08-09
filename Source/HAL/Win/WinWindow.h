//
//  BuiltInMaterialLayout.h
//  Engine
//
//  Created by Yongsik Im on 5/30/25.
//
#ifndef __HS_PLATFORM_WINDOW_WINDOWS_H__
#define __HS_PLATFORM_WINDOW_WINDOWS_H__

#include "Precompile.h"

#include "Engine/Core/Window.h"
#include "Engine/Platform/PlatformWindow.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <functional>

HS_NS_BEGIN

void hs_platform_window_set_pre_event_handler(std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> fnHandler);

HS_NS_END

#endif