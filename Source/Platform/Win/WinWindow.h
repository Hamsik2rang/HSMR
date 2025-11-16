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

void HS_API SetNativePreEventHandler(void* fnHandler);

bool HS_API CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow);
void HS_API DestroyNativeWindowInternal(NativeWindow& nativeWindow);
void HS_API ShowNativeWindowInternal(const NativeWindow& nativeWindow);
void HS_API PollNativeEventInternal(NativeWindow& nativeWindow);
void HS_API SetNativeWindowSizeInternal(uint16 width, uint16 height);
void HS_API GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight);

HS_NS_END

#endif