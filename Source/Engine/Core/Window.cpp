#include "Engine/Core/Window.h"

#include "Engine/Core/Log.h"

#include <SDL3/SDL.h>

HS_NS_BEGIN

Window::Window(const char* name, uint32 width, uint32 height, uint64 flags)
    : _name(name)
    , _width(width)
    , _height(height)
    , _flags(flags)
    , _resizable(false)
    , _shouldClose(false)
    , _useHDR(false)
    , _renderer(nullptr)
{}

Window::~Window()
{
    
}

uint32 Window::Initialize()
{
    SDL_Window* window = SDL_CreateWindow(_name, _width, _height, _flags);
    if (nullptr == window)
    {
        HS_LOG(error, "Window Initialize Failed");
        return HS_INVALID_WINDOW_ID;
    }

    _resizable = (_flags & SDL_WINDOW_RESIZABLE);
    _useHDR    = (_flags & SDL_WINDOW_HIGH_PIXEL_DENSITY);

    SDL_MetalView view = SDL_Metal_CreateView(window);

    _nativeHandle.window = static_cast<void*>(window);
    _nativeHandle.view   = static_cast<void*>(view);
    _nativeHandle.layer  = SDL_Metal_GetLayer(view);
}

void Window::NextFrame()
{
    onNextFrame();

    for (auto* child : _childs)
    {
        child->NextFrame();
    }
}

void Window::Update()
{
    onUpdate();

    for (auto* child : _childs)
    {
        child->Update();
    }
}

// Render는 Preorder로 수행
void Window::Render()
{
    if (nullptr != _renderer)
    {
        onRender();
    }

    for (auto* child : _childs)
    {
        child->Render();
    }
}

void Window::Present()
{
    onPresent();

    for (auto* child : _childs)
    {
        child->Present();
    }
}

// Shutdown은 Postorder로 진행
void Window::Shutdown()
{
    for (auto* child : _childs)
    {
        child->Shutdown();
    }

    _childs.clear();

    onShutdown();

    if (nullptr != _nativeHandle.view)
    {
        SDL_Metal_DestroyView(static_cast<SDL_MetalView>(_nativeHandle.view));
        _nativeHandle.view = nullptr;
    }
    if (nullptr != _nativeHandle.window)
    {
        SDL_DestroyWindow(static_cast<SDL_Window*>(_nativeHandle.window));
        _nativeHandle.window = nullptr;
    }
}

bool Window::PeekEvent(uint64 eventType, uint32 windowID)
{
    if (_id != windowID)
    {
        for (auto* child : _childs)
        {
            bool result = child->PeekEvent(eventType, windowID);
            if (result)
            {
                return true;
            }
        }

        return false;
    }

    PeekEvent(eventType, windowID);

    return true;
}

void Window::dispatchEvent(uint64 eventType)
{
    SDL_EventType event = static_cast<SDL_EventType>(eventType);

    switch (event)
    {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        {
            _shouldClose = true;
        }
        break;
        //...
        default:
            break;
    }
}

void Window::Flush()
{
    if (_shouldClose)
    {
        Shutdown();
    }

    for (auto* child : _childs)
    {
        child->Flush();
    }
}

HS_NS_END
