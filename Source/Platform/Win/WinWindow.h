//
//  BuiltInMaterialLayout.h
//  Engine
//
//  Created by Yongsik Im on 5/30/25.
//
#ifndef __HS_PLATFORM_WINDOW_WINDOWS_H__
#define __HS_PLATFORM_WINDOW_WINDOWS_H__

#include "Precompile.h"
#include "Core/Native/NativeWindow.h"

HS_NS_BEGIN

// SetNativePreEventHandler is already declared in Core/Native/NativeWindow.h (included above).

bool HS_CORE_API CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow);
void HS_CORE_API DestroyNativeWindowInternal(NativeWindow& nativeWindow);
void HS_CORE_API ShowNativeWindowInternal(const NativeWindow& nativeWindow);
void HS_CORE_API PollNativeEventInternal(NativeWindow& nativeWindow);
void HS_CORE_API SetNativeWindowSizeInternal(uint16 width, uint16 height);
void HS_CORE_API GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight);

HS_NS_END

#endif