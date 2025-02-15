#include "Engine/Core/Window.h"

#include "Engine/Core/Log.h"
#include "Engine/Core/Swapchain.h"
#include "Engine/RHI/RenderHandle.h"

#include <SDL3/SDL.h>

HS_NS_BEGIN

Window::Window(const char* name, uint32 width, uint32 height, uint64 flags)
    : _name(name)
    , _width(width)
    , _height(height)
    , _flags(flags)
    , _resizable(false)
    , _isClosed(false)
    , _shouldClose(false)
    , _useHDR(false)
    , _renderer(nullptr)
    , _scene(nullptr)
{}

Window::~Window()
{
    Shutdown();
}

uint32 Window::Initialize()
{
    SDL_Window* window = SDL_CreateWindow(_name, _width, _height, _flags);
    if (nullptr == window)
    {
        HS_LOG(error, "Window Initialize Failed");
        return HS_WINDOW_INVALID_ID;
    }

    _resizable = (_flags & SDL_WINDOW_RESIZABLE);
    _useHDR    = (_flags & SDL_WINDOW_HIGH_PIXEL_DENSITY);

    SDL_MetalView view = SDL_Metal_CreateView(window);

    _nativeHandle.window = static_cast<void*>(window);
    _nativeHandle.view   = static_cast<void*>(view);
    _nativeHandle.layer  = SDL_Metal_GetLayer(view);

    SDL_SetWindowSize(window, _width, _height);

    SwapchainInfo swInfo{};
    swInfo.nativeWindowHandle = static_cast<void*>(&_nativeHandle);
    swInfo.useDepth           = false;
    swInfo.useMSAA            = false;
    swInfo.useStencil         = false;

    _swapchain = new Swapchain(swInfo);

    onInitialize();

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);
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

void Window::Shutdown()
{
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

    _isClosed = true;
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

    // 트리 순회하면서 자식들 중에 close된 애들 해제 후 리스트에서 삭제
    std::list<Window*> deletedChilds(_childs.size());
    for (auto* child : _childs)
    {
        child->Flush();
        if (child->_isClosed)
        {
            deletedChilds.push_back(child);
        }
    }

    for (auto* child : deletedChilds)
    {
        for (auto* grandChild : child->_childs)
        {
            if (!grandChild->_isClosed)
            {
                _childs.push_back(grandChild);
            }
        }
        _childs.remove(child);
    }

    for (auto* delChild : deletedChilds)
    {
        delete delChild;
    }
}

HS_NS_END
