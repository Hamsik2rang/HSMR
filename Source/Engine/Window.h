//
//  Window.h
//  Engine
//
//  Created by Yongsik Im on 2025/2/7
//
#ifndef __HS_WINDOW_H__
#define __HS_WINDOW_H__

#include "Precompile.h"

#include "Core/Native/NativeWindow.h"

#include <list>
#include <queue>

/*#include "RHI/Swapchain.h"*/ namespace hs { class Swapchain; }
/*#include "Engine/Application.h"*/ namespace hs { class Application; }
namespace hs { class Renderer; }

HS_NS_BEGIN

#define HS_WINDOW_INVALID_ID HS_UINT32_MAX

class HS_API Window
{
public:
	Window(Application* ownerApp, const char* name, uint16 width, uint16 height, EWindowFlags flags);
	virtual ~Window();

	void ProcessEvent();
	void NextFrame();
	virtual void Update(float deltaTime);
	virtual void Render();
	virtual void Present();

	void Shutdown();
	void Flush();

	HS_FORCEINLINE const NativeWindow& GetNativeWindow() { return _nativeWindow; }
	HS_FORCEINLINE const char* GetName() { return _nativeWindow.title; }
	HS_FORCEINLINE bool IsOpened() { return !_shouldClose; }
	HS_FORCEINLINE bool IsMinimize() { return _nativeWindow.isMinimized; }
	HS_FORCEINLINE void Close() { _shouldClose = true; }
	HS_FORCEINLINE float GetScale() { return _nativeWindow.scale; }

	Application* GetApplication();

	void SetPreEventHandler(void* handler);

protected:

	virtual bool onInitialize() { return true; }
	virtual void onNextFrame() {}
	virtual void onUpdate(float deltaTime) {}
	virtual void onRender() {}
	virtual void onPresent() {}
	virtual void onShutdown() {}
	virtual void onResize() {}
	virtual void onSuspend() {}
	virtual void onRestore() {}

	std::list<Window*> _childs;
	Window* _parent = nullptr;

	Swapchain* _swapchain;

	NativeWindow _nativeWindow;

	Application* _ownerApp;

	// For windows
	void* _preEventHandler;

	bool _shouldClose;
	bool _shouldUpdate;
	bool _shouldPresent;
	bool _isClosed;
};

HS_NS_END



#endif /* Window_h */
