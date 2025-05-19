#include "Engine/Core/Window.h"

#include "Engine/Core/Log.h"
#include "Engine/RHI/Swapchain.h"
#include "Engine/RHI/RenderHandle.h"

HS_NS_BEGIN

Window::Window(const char* name, uint16 width, uint16 height, EWindowFlags flags)
    : _isClosed(false)
    , _shouldClose(false)
    , _renderer(nullptr)
    , _scene(nullptr)
{
    if (hs_platform_window_create(name, width, height, flags, _nativeWindow))
    {
        HS_LOG(crash, "Fail to create NativeWindow");
    }

    _rhiContext = hs_engine_get_rhi_context();

    SwapchainInfo swInfo{};
    swInfo.nativeWindow = &_nativeWindow;
    swInfo.useDepth     = false;
    swInfo.useMSAA      = false;
    swInfo.useStencil   = false;

    _swapchain = _rhiContext->CreateSwapchain(swInfo);

    onInitialize();

    // SetWindowPos
    // ShowWindow
}

Window::~Window()
{
    Shutdown();
}

// uint32 Window::Initialize()
//{
//     SDL_Window* window = SDL_CreateWindow(_name, _width, _height, _flags);
//     if (nullptr == window)
//     {
//         HS_LOG(error, "Window Initialize Failed");
//         return HS_WINDOW_INVALID_ID;
//     }
//     _scale = SDL_GetWindowDisplayScale(window);
//     SDL_SetWindowSize(window, _width / _scale, _height / _scale);
//
//     _resizable = (_flags & SDL_WINDOW_RESIZABLE);
//     _useHDR    = (_flags & SDL_WINDOW_HIGH_PIXEL_DENSITY);
//
//     SDL_MetalView view = SDL_Metal_CreateView(window);
//
//     hs_platform_create_window(_nativeWindow, _name, _width, _height, _flags);
//
//     _rhiContext = hs_engine_get_rhi_context();
//
//     SwapchainInfo swInfo{};
//     swInfo.width              = _nativeWindow.width;
//     swInfo.height             = _nativeWindow.height;
//     swInfo.NativeWindow = static_cast<void*>(&_nativeWindow);
//     swInfo.useDepth           = false;
//     swInfo.useMSAA            = false;
//     swInfo.useStencil         = false;
//
//     _swapchain = _rhiContext->CreateSwapchain(swInfo);
//
//     onInitialize();
//
//     SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
//     SDL_ShowWindow(window);
//
//     return 0;
// }

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

    hs_platform_window_destroy(_nativeWindow);

    _isClosed = true;
}

void Window::ProcessEvent()
{
    EWindowEvent event;
    while (hs_platform_window_peek_event(&_nativeWindow, event))
    {
        switch (event)
        {
            case EWindowEvent::OPEN:
                _shouldClose = false;

                break;
            case EWindowEvent::CLOSE:
                _shouldClose   = true;
                _shouldUpdate  = false;
                _shouldPresent = false;

                break;
            case EWindowEvent::MAXIMIZE:
                _shouldUpdate  = true;
                _shouldPresent = true;
                onRestore();
                break;
            case EWindowEvent::MINIMIZE:
                _shouldUpdate  = false;
                _shouldPresent = false;
                break;
            case EWindowEvent::NONE:

                break;
            case EWindowEvent::RESIZE_ENTER:
                _shouldUpdate  = false;
                _shouldPresent = false;
                break;
            case EWindowEvent::RESIZE_EXIT:
                _shouldUpdate  = true;
                _shouldPresent = true;
                break;
            case EWindowEvent::MOVE:

                break;
            case EWindowEvent::FOCUS_IN:

                break;
            case EWindowEvent::FOCUS_OUT:

                break;
            case EWindowEvent::RESTORE:

                break;
        }
    }

    for (auto* child : _childs)
    {
        child->ProcessEvent();
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
