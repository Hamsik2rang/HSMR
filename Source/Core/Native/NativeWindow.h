//
//  NativeWindow.h
//  Platform
//
//  Created by Yongsik Im on 5/16/2025
//

#ifndef __HS_NATIVE_WINDOW_H__
#define __HS_NATIVE_WINDOW_H__

#include "Precompile.h"

#include "Core/Native/NativeEvent.h"

HS_NS_BEGIN

// Same with SDL_WindowFlags.
enum class HS_CORE_API EWindowFlags : uint64
{
	None = 0,

	Fullscreen        = HS_BIT(0),  /**< window is in fullscreen mode */
	Opengl            = HS_BIT(1),  /**< window usable with OpenGL context */
	Occluded          = HS_BIT(2),
	Hidden            = HS_BIT(3),  /**< window is neither mapped onto the desktop nor shown in the taskbar/dock/window list; SDL_ShowWindow() is required for it to become visible */
	Borderless        = HS_BIT(4),  /**< no window decoration */
	Resizable         = HS_BIT(5),  /**< window can be resized */
	Minimized         = HS_BIT(6),  /**< window is minimized */
	Maximized         = HS_BIT(7),  /**< window is maximized */
	MouseGrabbed      = HS_BIT(8),  /**< window has grabbed mouse input */
	InputFocus        = HS_BIT(9),  /**< window has input focus */
	MouseFocus        = HS_BIT(10), /**< window has mouse focus */
	External          = HS_BIT(11), /**< window not created by SDL */
	Modal             = HS_BIT(12), /**< window is modal */
	HighPixelDensity  = HS_BIT(13), /**< window uses high pixel density back buffer if possible */
	MouseCapture      = HS_BIT(14), /**< window has mouse captured (unrelated to MouseGrabbed) */
	MouseRelativeMode = HS_BIT(15), /**< window has relative mode enabled */
	AlwaysOnTop       = HS_BIT(16), /**< window should always be above others */
	Utility           = HS_BIT(17), /**< window should be treated as a utility window, not showing in the task bar and window list */
	Tooltip           = HS_BIT(18), /**< window should be treated as a tooltip and does not get mouse or keyboard focus, requires a parent window */
	PopupMenu         = HS_BIT(19), /**< window should be treated as a popup menu, requires a parent window */
	KeyboardGrabbed   = HS_BIT(20), /**< window has grabbed keyboard input */
	Vulkan            = HS_BIT(28), /**< window usable for Vulkan surface */
	Metal             = HS_BIT(29), /**< window usable for Metal view */
	Transparent       = HS_BIT(30), /**< window with transparent buffer */
	NotFocusable      = HS_BIT(31), /**< window should not be focusable */
};

constexpr HS_CORE_API  EWindowFlags operator&(EWindowFlags lhs, EWindowFlags rhs)
{
	return static_cast<EWindowFlags>(static_cast<uint64>(lhs) & static_cast<uint64>(rhs));
}

constexpr HS_CORE_API  EWindowFlags& operator&=(EWindowFlags& lhs, EWindowFlags rhs)
{
	lhs = lhs & rhs;
	return lhs;
}

constexpr HS_CORE_API EWindowFlags operator|(EWindowFlags lhs, EWindowFlags rhs)
{
	return static_cast<EWindowFlags>(static_cast<uint64>(lhs) | static_cast<uint64>(rhs));
}

constexpr HS_CORE_API EWindowFlags operator|(EWindowFlags lhs, uint64 rhs)
{
	return lhs | static_cast<EWindowFlags>(rhs);
}

constexpr HS_CORE_API EWindowFlags& operator|=(EWindowFlags& lhs, EWindowFlags rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

struct HS_CORE_API NativeWindow
{
	EWindowFlags flags;
	void* handle; // HWND for Windows, NSWindow for macOS, SDL_Window* for SDL

	// Graphics layer handles (platform/API specific)
	void* graphicsLayer; // CAMetalLayer* (Metal), nullptr (Vulkan)
	void* graphicsView;  // NSView*/SDL_MetalView (Metal), nullptr (Vulkan)

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
	
	bool futureUse : 4; // padding.
};

bool HS_CORE_API CreateNativeWindow(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow);
void HS_CORE_API DestroyNativeWindow(NativeWindow& nativeWindow);
void HS_CORE_API ShowNativeWindow(const NativeWindow& nativeWindow);
void HS_CORE_API PollNativeEvent(NativeWindow& nativeWindow);
void HS_CORE_API SetNativeWindowSize(uint16 width, uint16 height);
void HS_CORE_API GetNativeWindowSize(uint16& outWidth, uint16& outHeight);

bool HS_CORE_API PeekNativeEvent(hs::NativeWindow* pWindow, NativeEvent& outEvent);
void HS_CORE_API PushNativeEvent(const hs::NativeWindow* pWindow, NativeEvent event);
NativeEvent HS_CORE_API PopNativeEvent(const hs::NativeWindow* pWindow);

void HS_CORE_API SetNativePreEventHandler(void* fnHandler);

HS_NS_END

#endif /*__HS_PLATFORM_WINDOW__*/
