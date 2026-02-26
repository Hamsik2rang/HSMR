#include "Editor/Core/SimpleApplication.h"

#include "Core/Log.h"
#include "Core/HAL/Timer.h"
#include "Core/Native/NativeWindow.h"

#include "Resource/ObjectManager.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/SimpleWindow.h"

#if defined(__APPLE__)
#include "Platform/Mac/AutoReleasePool.h"
#endif

HS_NS_EDITOR_BEGIN

SimpleApplication::SimpleApplication(const char* appName) noexcept
    : Application(appName)
    , _guiContext(nullptr)
    , _deltaTime(0.0f)
{
    _guiContext = new GUIContext();
    ObjectManager::Initialize();
}

SimpleApplication::~SimpleApplication()
{
    Shutdown();
}

void SimpleApplication::Shutdown()
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

    ObjectManager::Finalize();
}

void SimpleApplication::Run()
{
    Timer::Start();

    EWindowFlags windowFlags = EWindowFlags::None;
    windowFlags |= EWindowFlags::Resizable;
    windowFlags |= EWindowFlags::HighPixelDensity;
#if defined(__APPLE__)
    windowFlags |= EWindowFlags::Metal;
#else
    windowFlags |= EWindowFlags::Vulkan;
#endif

    _window = new SimpleWindow(this, "HSMR Simple", 1920, 1080, windowFlags);
    if (nullptr == _window->GetNativeWindow().handle)
    {
        HS_LOG(error, "Failed to create simple window");
        return;
    }

    ShowNativeWindow(_window->GetNativeWindow());

    float lastTime = Timer::GetElapsedMilliseconds();

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

        float curTime = Timer::GetElapsedMilliseconds();
        _deltaTime    = (curTime - lastTime) / 1000.0f;
        lastTime      = curTime;

        _window->NextFrame();
        _window->Update(_deltaTime);
        _window->Render();
        _window->Present();
        _window->Flush();
    }
}

GUIContext* SimpleApplication::GetGUIContext()
{
    return _guiContext;
}

HS_NS_EDITOR_END
