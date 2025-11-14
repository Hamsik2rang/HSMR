#include "Editor/Core/EditorApplication.h"

#include "Core/Log.h"
#include "Core/SystemContext.h"
#include "Core/Native/NativeWindow.h"

#include "Engine/Resource/ObjectManager.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorWindow.h"

#if defined(__APPLE__)
#include "Platform/Mac/AutoReleasePool.h"
#endif

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

    _window = new EditorWindow(this, "EditorApp BaseWindow", 1280, 720, windowFlags);
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
    // TODO: Elapse Timer

    while (true)
    {
#if defined(__APPLE__)
        AutoReleasePool pool;
#endif
        _window->ProcessEvent();
        
        if(!_window->IsOpened())
        {
            break;
        }
        
        _window->NextFrame();

        _window->Update();

        _window->Render();

        _window->Present();

        _window->Flush();
    }

    Shutdown();
}

GUIContext* EditorApplication::GetGUIContext()
{
    return _guiContext;
}

HS_NS_EDITOR_END
