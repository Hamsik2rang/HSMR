#include "Editor/Core/EditorApplication.h"

#include "Core/Log.h"
#include "Core/SystemContext.h"
#include "Core/Native/NativeWindow.h"
#include "Core/HAL/Timer.h"

#include "Engine/EngineContext.h"
#include "Engine/Resource/ObjectManager.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorWindow.h"

#if defined(__APPLE__)
#include "Platform/Mac/AutoReleasePool.h"
#endif

#include <thread>
#include <chrono>

HS_NS_EDITOR_BEGIN

EditorApplication::EditorApplication(const char* appName) noexcept
	: Application(appName)
	, _guiContext(nullptr)
	, _deltaTime(0.0f)
{
	_guiContext = new GUIContext();

	ObjectManager::Initialize();

	EWindowFlags windowFlags = EWindowFlags::NONE;
	windowFlags |= EWindowFlags::WINDOW_RESIZABLE;
	windowFlags |= EWindowFlags::WINDOW_HIGH_PIXEL_DENSITY;
	windowFlags |= EWindowFlags::WINDOW_METAL;

	_window = new EditorWindow(this, "EditorApp BaseWindow", 1920, 1080, windowFlags);
	if (nullptr == _window->GetNativeWindow().handle)
	{
		HS_LOG(error, "Fail to initialize base window");
	}

	ShowNativeWindow(_window->GetNativeWindow());
}

EditorApplication::~EditorApplication()
{
	Shutdown();
}

void EditorApplication::Shutdown()
{
	if (_window && _window->IsOpened())
	{
		_window->Shutdown();
		delete _window;
		_window = nullptr;
	}

	if (_guiContext)
	{
		_guiContext->Finalize();
		delete _guiContext;
		_guiContext = nullptr;
	}
	//...

	ObjectManager::Finalize();
}

void EditorApplication::Run()
{
	Timer::Start();

	double lastTimeMs        = 0.0;
	const bool enableVSync   = true;  // TODO: Get from config
	const double targetFps   = 60.0;
	const double targetFrameTimeMs = 1000.0 / targetFps;

	while (true)
	{
#if defined(__APPLE__)
		AutoReleasePool pool;
#endif
		_window->ProcessEvent();

		if (!_window->IsOpened())
		{
			break;
		}

		double currentTimeMs = Timer::GetElapsedMilliseconds();
		_deltaTime           = static_cast<float>((currentTimeMs - lastTimeMs) * 0.001);
		lastTimeMs           = currentTimeMs;

		_window->NextFrame();

		_window->Update();

		_window->Render();

		_window->Present();

		_window->Flush();

		// Frame rate limiting when VSync is disabled
		if (!enableVSync)
		{
			double frameTimeMs   = Timer::GetElapsedMilliseconds() - currentTimeMs;
			double sleepTimeMs   = targetFrameTimeMs - frameTimeMs;

			if (sleepTimeMs > 1.0)
			{
				// Sleep for most of the remaining time to save CPU/power
				std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTimeMs - 1.0)));
			}
		}
		// When VSync is enabled, Present() blocks until the next vertical blank,
		// so no additional frame limiting is needed.
	}

	Timer::Stop();
	Shutdown();
}

GUIContext* EditorApplication::GetGUIContext()
{
	return _guiContext;
}

HS_NS_EDITOR_END
