//
//  NativeWindow.h
//  HAL
//
//  Created by Yongsik Im on 5/16/2025
//

#ifndef __HS_NATIVE_WINDOW_H__
#define __HS_NATIVE_WINDOW_H__

#include "Precompile.h"

#include "HAL/NativeEvent.h"

HS_NS_BEGIN

// Same with SDL_WindowFlags.
enum class EWindowFlags : uint64
{
	NONE = 0,

	WINDOW_FULLSCREEN = HS_BIT(0), /**< window is in fullscreen mode */
	WINDOW_OPENGL = HS_BIT(1), /**< window usable with OpenGL context */
	WINDOW_OCCLUDED = HS_BIT(2),
	WINDOW_HIDDEN = HS_BIT(3),  /**< window is neither mapped onto the desktop nor shown in the taskbar/dock/window list; SDL_ShowWindow() is required for it to become visible */
	WINDOW_BORDERLESS = HS_BIT(4),  /**< no window decoration */
	WINDOW_RESIZABLE = HS_BIT(5),  /**< window can be resized */
	WINDOW_MINIMIZED = HS_BIT(6),  /**< window is minimized */
	WINDOW_MAXIMIZED = HS_BIT(7),  /**< window is maximized */
	WINDOW_MOUSE_GRABBED = HS_BIT(8),  /**< window has grabbed mouse input */
	WINDOW_INPUT_FOCUS = HS_BIT(9),  /**< window has input focus */
	WINDOW_MOUSE_FOCUS = HS_BIT(10), /**< window has mouse focus */
	WINDOW_EXTERNAL = HS_BIT(11), /**< window not created by SDL */
	WINDOW_MODAL = HS_BIT(12), /**< window is modal */
	WINDOW_HIGH_PIXEL_DENSITY = HS_BIT(13), /**< window uses high pixel density back buffer if possible */
	WINDOW_MOUSE_CAPTURE = HS_BIT(14), /**< window has mouse captured (unrelated to MOUSE_GRABBED) */
	WINDOW_MOUSE_RELATIVE_MODE = HS_BIT(15), /**< window has relative mode enabled */
	WINDOW_ALWAYS_ON_TOP = HS_BIT(16), /**< window should always be above others */
	WINDOW_UTILITY = HS_BIT(17), /**< window should be treated as a utility window, not showing in the task bar and window list */
	WINDOW_TOOLTIP = HS_BIT(18), /**< window should be treated as a tooltip and does not get mouse or keyboard focus, requires a parent window */
	WINDOW_POPUP_MENU = HS_BIT(19), /**< window should be treated as a popup menu, requires a parent window */
	WINDOW_KEYBOARD_GRABBED = HS_BIT(20), /**< window has grabbed keyboard input */
	WINDOW_VULKAN = HS_BIT(28), /**< window usable for Vulkan surface */
	WINDOW_METAL = HS_BIT(29), /**< window usable for Metal view */
	WINDOW_TRANSPARENT = HS_BIT(30), /**< window with transparent buffer */
	WINDOW_NOT_FOCUSABLE = HS_BIT(31), /**< window should not be focusable */
};

constexpr EWindowFlags operator&(EWindowFlags lhs, EWindowFlags rhs)
{
	return static_cast<EWindowFlags>(static_cast<uint64>(lhs) & static_cast<uint64>(rhs));
}

constexpr EWindowFlags& operator&=(EWindowFlags& lhs, EWindowFlags rhs)
{
	lhs = lhs & rhs;
	return lhs;
}

constexpr EWindowFlags operator|(EWindowFlags lhs, EWindowFlags rhs)
{
	return static_cast<EWindowFlags>(static_cast<uint64>(lhs) | static_cast<uint64>(rhs));
}

constexpr EWindowFlags operator|(EWindowFlags lhs, uint64 rhs)
{
	return lhs | static_cast<EWindowFlags>(rhs);
}

constexpr EWindowFlags& operator|=(EWindowFlags& lhs, EWindowFlags rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

struct NativeWindow
{
	EWindowFlags flags;
	void* handle; // HWND for Windows, NSWindow for macOS, etc.

	const char* title;
	uint16 width;
	uint16 height;
	uint16 surfaceWidth;
	uint16 surfaceHeight;

	float scale = 1.0f;

	bool isMinimized : 1;
	bool isMaximized : 1;
	bool resizable : 1;
	bool useHDR : 1;

	bool shouldUpdate : 1;
	bool shouldRender : 1;
	bool shouldClose : 1;
	bool futureUse : 1; // padding.
};

bool CreateNativeWindow(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow);
void DestroyNativeWindow(NativeWindow& nativeWindow);
void ShowNativeWindow(const NativeWindow& nativeWindow);
void PollNativeEvent(NativeWindow& nativeWindow);
void SetNativeWindowSize(uint16 width, uint16 height);
void GetNativeWindowSize(uint16& outWidth, uint16& outHeight);

bool PeekNativeEvent(HS::NativeWindow* pWindow, NativeEvent& outEvent);
void PushNativeEvent(const HS::NativeWindow* pWindow, NativeEvent event);
NativeEvent PopNativeEvent(const HS::NativeWindow* pWindow);

void SetNativePreEventHandler(void* fnHandler);

HS_NS_END

#endif /*__HS_PLATFORM_WINDOW__*/
