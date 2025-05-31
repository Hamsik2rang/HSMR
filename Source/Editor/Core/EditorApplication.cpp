#include "Editor/Core/EditorApplication.h"

#include "Engine/Core/Log.h"
#include "Engine/Platform/PlatformApplication.h"
#include "Engine/Platform/PlatformWindow.h"
#include "Engine/Utility/ResourceManager.h"


#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorWindow.h"

#if defined(__APPLE__)
#include "Engine/Platform/Mac/AutoReleasePool.h"
#endif

HS_NS_EDITOR_BEGIN

EditorApplication::EditorApplication(const char* appName) noexcept
    : Application(appName)
	, _isInitialized(false)
    , _guiContext(nullptr)
    , _deltaTime(0.0f)
{
}

EditorApplication::~EditorApplication()
{
    Finalize();
}

bool EditorApplication::Initialize(EngineContext* engineContext)
{
    if (_isInitialized)
    {
        HS_LOG(error, "Application is already initialized");
        return true;
    }
    _engineContext = engineContext;
    hs_engine_set_context(engineContext);
    
    _guiContext = new GUIContext();
    _guiContext->Initialize();
    
    ResourceManager::Initialize();

    EWindowFlags windowFlags = EWindowFlags::NONE;
    windowFlags |= EWindowFlags::WINDOW_RESIZABLE;
    windowFlags |= EWindowFlags::WINDOW_HIGH_PIXEL_DENSITY;
    windowFlags |= EWindowFlags::WINDOW_METAL;

    _window = new EditorWindow("EditorApp BaseWindow", 1280, 720, windowFlags);
    if (nullptr == _window->GetNativeWindow().handle)
    {
        HS_LOG(error, "Fail to initialize base window");
        return false;
    }

    hs_platform_window_show(_window->GetNativeWindow());
    

    _isInitialized = true;

    return _isInitialized;
}

void EditorApplication::Finalize()
{
    if(false == _isInitialized)
    {
        return;
    }
    
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

    hs_platform_shutdown(this);
}

void EditorApplication::Run()
{
    HS_ASSERT(_isInitialized, "Application isn't initialized");

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

    Finalize();
}

HS_NS_EDITOR_END
