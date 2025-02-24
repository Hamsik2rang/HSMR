#include "Editor/Core/EditorApplication.h"

#include "Engine/Core/Log.h"

#include "Editor/GUI/GUIContext.h"
#include "Editor/Core/EditorWindow.h"

#include "ImGui/backends/imgui_impl_sdl3.h"

HS_NS_EDITOR_BEGIN

EditorApplication::EditorApplication(const std::string& appName)
    : Application(appName)
{}

EditorApplication::~EditorApplication()
{
    
}

bool EditorApplication::Initialize(EngineContext* engineContext)
{
    _engineContext = engineContext;
    hs_engine_set_context(engineContext);
    if (_isInitialized)
    {
        HS_LOG(error, "Application is already initialized");
        return true;
    }

    _guiContext = new GUIContext();
    _guiContext->Initialize();

    Window::EFlags windowFlags = Window::EFlags::NONE;
    windowFlags |= Window::EFlags::WINDOW_RESIZABLE;
    windowFlags |= Window::EFlags::WINDOW_HIGH_PIXEL_DENSITY;
    windowFlags |= Window::EFlags::WINDOW_METAL;

    _window = new EditorWindow("EditorApp BaseWindow", 1600, 1050, static_cast<uint64>(windowFlags));
    if (HS_WINDOW_INVALID_ID == _window->Initialize())
    {
        HS_LOG(error, "Fail to initialize base window");
        return false;
    }

    //...

    _isInitialized = true;

    return _isInitialized;
}

void EditorApplication::Finalize()
{
    
    if (_window)
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
}

void EditorApplication::Run()
{
    HS_ASSERT(_isInitialized, "Application isn't initialized");

    // TODO: Elapse Timer

    while (_window->IsOpened())
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            _window->PeekEvent(event.type, event.window.windowID);
        }

        _window->NextFrame();

        _window->Update();

        _window->Render();

        _window->Present();

        _window->Flush();
    }
}

HS_NS_EDITOR_END
