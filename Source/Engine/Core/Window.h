//
//  Window.h
//  HSMR
//
//  Created by Yongsik Im on 2025.2.7
//
#ifndef __HS_WINDOW_H__
#define __HS_WINDOW_H__

#include "Precompile.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Renderer/RenderDefinition.h"

#include "Engine/Platform/PlatformWindow.h"

#include <list>
#include <queue>

HS_NS_BEGIN

#define HS_WINDOW_INVALID_ID HS_UINT32_MAX

class Renderer;
class Scene;
class Swapchain;
class RHIContext;

class Window
{
public:
    Window(const char* name, uint16 width, uint16 height, EWindowFlags flags);
    virtual ~Window();

    virtual void ProcessEvent();
    virtual void NextFrame();
    virtual void Update();
    virtual void Render();
    virtual void Present();

    void Shutdown();
    void Flush();

    HS_FORCEINLINE const NativeWindow& GetNativeWindow() { return _nativeWindow; }
    HS_FORCEINLINE const char*         GetName() { return _nativeWindow.title; }
    HS_FORCEINLINE bool                IsOpened() { return !_shouldClose; }
    HS_FORCEINLINE bool                IsMinimize() { return _nativeWindow.isMinimized; }
    HS_FORCEINLINE void                Close() { _shouldClose = true; }
    HS_FORCEINLINE Swapchain*          GetSwapchain() { return _swapchain; }
    HS_FORCEINLINE float               GetScale() { return _nativeWindow.scale; }

protected:
    virtual bool onInitialize() { return true; }
    virtual void onNextFrame() {}
    virtual void onUpdate() {}
    virtual void onRender() {}
    virtual void onPresent() {}
    virtual void onShutdown() {}
    virtual void onResize() {}
    virtual void onSuspend() {}
    virtual void onRestore() {}

    std::list<Window*> _childs;
    Window*            _parent = nullptr;

    NativeWindow _nativeWindow;

    RHIContext* _rhiContext = nullptr;
    Renderer*   _renderer   = nullptr;
    Scene*      _scene      = nullptr;

    Swapchain* _swapchain = nullptr;

    bool _shouldClose;
    bool _shouldUpdate;
    bool _shouldPresent;
    bool _isClosed;
};

HS_NS_END


bool hs_window_peek_event(HS::NativeWindow* pWindow, HS::EWindowEvent& outEvent, bool remove);
void hs_window_push_event(const HS::NativeWindow* pWindow, HS::EWindowEvent event);
HS::EWindowEvent hs_window_pop_event(const HS::NativeWindow* pWindow);


#endif /* Window_h */
