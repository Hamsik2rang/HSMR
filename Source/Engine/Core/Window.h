//
//  Window.h
//  HSMR
//
//  Created by Yongsik Im on 2/7/25.
//
#ifndef __HS_WINDOW_H__
#define __HS_WINDOW_H__

#include "Precompile.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Renderer/RenderDefinition.h"

#include <list>

HS_NS_BEGIN

#define HS_WINDOW_INVALID_ID HS_UINT32_MAX

class Renderer;
class Scene;
class Swapchain;
class RHIContext;

struct NativeWindowHandle
{
    void* window;
    void* view;
    void* layer;
};

class Window
{
public:
    // Same with SDL_WindowFlags.
    enum class EFlags : uint64
    {
        NONE = 0,

        WINDOW_FULLSCREEN          = HS_BIT(0), /**< window is in fullscreen mode */
        WINDOW_OPENGL              = HS_BIT(1), /**< window usable with OpenGL context */
        WINDOW_OCCLUDED            = HS_BIT(2),
        WINDOW_HIDDEN              = HS_BIT(3),  /**< window is neither mapped onto the desktop nor shown in the taskbar/dock/window list; SDL_ShowWindow() is required for it to become visible */
        WINDOW_BORDERLESS          = HS_BIT(4),  /**< no window decoration */
        WINDOW_RESIZABLE           = HS_BIT(5),  /**< window can be resized */
        WINDOW_MINIMIZED           = HS_BIT(6),  /**< window is minimized */
        WINDOW_MAXIMIZED           = HS_BIT(7),  /**< window is maximized */
        WINDOW_MOUSE_GRABBED       = HS_BIT(8),  /**< window has grabbed mouse input */
        WINDOW_INPUT_FOCUS         = HS_BIT(9),  /**< window has input focus */
        WINDOW_MOUSE_FOCUS         = HS_BIT(10), /**< window has mouse focus */
        WINDOW_EXTERNAL            = HS_BIT(11), /**< window not created by SDL */
        WINDOW_MODAL               = HS_BIT(12), /**< window is modal */
        WINDOW_HIGH_PIXEL_DENSITY  = HS_BIT(13), /**< window uses high pixel density back buffer if possible */
        WINDOW_MOUSE_CAPTURE       = HS_BIT(14), /**< window has mouse captured (unrelated to MOUSE_GRABBED) */
        WINDOW_MOUSE_RELATIVE_MODE = HS_BIT(15), /**< window has relative mode enabled */
        WINDOW_ALWAYS_ON_TOP       = HS_BIT(16), /**< window should always be above others */
        WINDOW_UTILITY             = HS_BIT(17), /**< window should be treated as a utility window, not showing in the task bar and window list */
        WINDOW_TOOLTIP             = HS_BIT(18), /**< window should be treated as a tooltip and does not get mouse or keyboard focus, requires a parent window */
        WINDOW_POPUP_MENU          = HS_BIT(19), /**< window should be treated as a popup menu, requires a parent window */
        WINDOW_KEYBOARD_GRABBED    = HS_BIT(20), /**< window has grabbed keyboard input */
        WINDOW_VULKAN              = HS_BIT(28), /**< window usable for Vulkan surface */
        WINDOW_METAL               = HS_BIT(29), /**< window usable for Metal view */
        WINDOW_TRANSPARENT         = HS_BIT(30), /**< window with transparent buffer */
        WINDOW_NOT_FOCUSABLE       = HS_BIT(31), /**< window should not be focusable */
    };

    Window(const char* name, uint32 width, uint32 height, uint64 flags);
    virtual ~Window();

    virtual void NextFrame();
    virtual void Update();
    virtual void Render();
    virtual void Present();

    uint32 Initialize();
    void   Shutdown();
    void   Flush();

    HS_FORCEINLINE const NativeWindowHandle& GetNativeHandle() { return _nativeHandle; }
    HS_FORCEINLINE const char*               GetName() { return _name; }
    HS_FORCEINLINE uint32                    GetWindowID() { return _id; }
    HS_FORCEINLINE uint64                    GetFlags() { return _flags; }
    HS_FORCEINLINE bool                      PeekEvent(uint64 eventType, uint32 windowID);
    HS_FORCEINLINE bool                      IsOpened() { return !_shouldClose; }
    HS_FORCEINLINE void                      Close() { _shouldClose = true; }
    HS_FORCEINLINE Swapchain*                GetSwapchain() { return _swapchain; }

protected:
    virtual void dispatchEvent(uint64 eventType);

    virtual bool onInitialize(){};
    virtual void onNextFrame() {}
    virtual void onUpdate() {}
    virtual void onRender() {}
    virtual void onPresent() {}
    virtual void onShutdown() {}

    std::list<Window*> _childs;
    Window*            _parent = nullptr;

    NativeWindowHandle _nativeHandle;

    RHIContext* _rhiContext = nullptr;
    Renderer* _renderer = nullptr;
    Scene*    _scene    = nullptr;
    
    Swapchain* _swapchain = nullptr;

    const char* _name;
    uint32      _width;
    uint32      _height;
    uint64      _flags;
    uint32      _id;

    bool _shouldClose;
    bool _isClosed;
    bool _resizable;
    bool _useHDR;
};

constexpr Window::EFlags operator&(Window::EFlags lhs, Window::EFlags rhs)
{
    return static_cast<Window::EFlags>(static_cast<uint64>(lhs) & static_cast<uint64>(rhs));
}

constexpr Window::EFlags& operator&=(Window::EFlags& lhs, Window::EFlags rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

constexpr Window::EFlags operator|(Window::EFlags lhs, Window::EFlags rhs)
{
    return static_cast<Window::EFlags>(static_cast<uint64>(lhs) | static_cast<uint64>(rhs));
}

constexpr Window::EFlags operator|(Window::EFlags lhs, uint64 rhs)
{
    return lhs | static_cast<Window::EFlags>(rhs);
}

constexpr Window::EFlags& operator|=(Window::EFlags& lhs, Window::EFlags rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

HS_NS_END

#endif /* Window_h */
