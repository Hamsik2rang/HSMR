//
//  MacWindow.h
//  HAL
//
//  Created by Yongsik Im on 8/9/2025
//
#ifndef __HS_MAC_WINDOW_H__
#define __HS_MAC_WINDOW_H__

#include "Precompile.h"

#include "HAL/NativeWindow.h"


HS_NS_BEGIN


bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow);
void DestroyNativeWindowInternal(NativeWindow& nativeWindow);
void ShowNativeWindowInternal(const NativeWindow& nativeWindow);
void PollNativeEventInternal(NativeWindow& nativeWindow);
void SetNativeWindowSizeInternal(uint16 width, uint16 height);
void GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight);


HS_NS_END

#endif /*__HS_PLATFORM_OSX_H__*/
