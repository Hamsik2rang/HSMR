//
//  SDLPlatform.h
//  Platform
//
//  Created for HSMR Lightweight Prototyping Framework
//

#ifndef __HS_SDL_PLATFORM_H__
#define __HS_SDL_PLATFORM_H__

#include "Precompile.h"

#ifdef __SDL__

#include "Core/Native/NativeWindow.h"

struct SDL_Window;

HS_NS_BEGIN

// Get the SDL window handle for Vulkan surface creation
SDL_Window* GetSDLWindow();

// Internal platform functions (called by NativeWindow.cpp)
bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow);
void DestroyNativeWindowInternal(NativeWindow& nativeWindow);
void ShowNativeWindowInternal(const NativeWindow& nativeWindow);
void PollNativeEventInternal(NativeWindow& nativeWindow);
void SetNativeWindowSizeInternal(uint16 width, uint16 height);
void GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight);

HS_NS_END

#endif // __SDL__

#endif // __HS_SDL_PLATFORM_H__
