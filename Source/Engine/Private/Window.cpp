#include "Engine/Window.h"

#include "Core/Native/NativeEvent.h"
#include "Core/Log.h"

#include "RHI/Swapchain.h"
#include "RHI/RHIContext.h"

#include "Engine/Application.h"

#include <queue>

HS_NS_BEGIN

Window::Window(Application* ownerApp, const char* name, uint16 width, uint16 height, EWindowFlags flags)
    : _isClosed(false)
    , _shouldClose(false)
    , _shouldUpdate(true)
    , _shouldPresent(true)
    , _ownerApp(ownerApp)
    , _preEventHandler(nullptr)
    , _rhiContext(RHIContext::Get())
{
    if (!CreateNativeWindow(name, width, height, flags, _nativeWindow))
    {
        HS_LOG(crash, "Fail to create NativeWindow");
    }
    
    SwapchainInfo scInfo{};
    scInfo.nativeWindow = &_nativeWindow;
    scInfo.useDepth = false;
    scInfo.useMSAA = false;
    scInfo.useStencil = false;
    scInfo.enableVSync  = true;

    _swapchain = _rhiContext->CreateSwapchain(scInfo);
    _renderTargets.resize(_swapchain->GetMaxFrameCount());

    for (size_t i = 0; i < _renderTargets.size(); i++)
    {
        RenderTargetInfo info{};
        info.width = width;
        info.height = height;
        info.colorTextureCount = 1;
        info.colorTextureInfos.resize(info.colorTextureCount);
        for (size_t j = 0; j < info.colorTextureCount; j++)
        {
            info.colorTextureInfos[j].arrayLength = 1;
            info.colorTextureInfos[j].extent.width  = width;
            info.colorTextureInfos[j].extent.height = height;
            info.colorTextureInfos[j].extent.depth = 1;
            info.colorTextureInfos[j].format        = EPixelFormat::R8G8B8A8Srgb;
            info.colorTextureInfos[j].usage         = ETextureUsage::ColorAttachment | ETextureUsage::Staging | ETextureUsage::Sampled;
            info.colorTextureInfos[j].isCompressed  = false;
            info.colorTextureInfos[j].byteSize      = 4 * width * height * 1 /*depth*/;
        }

        info.useDepthStencilTexture                = true;
        info.depthStencilInfo.extent.width         = width;
        info.depthStencilInfo.extent.height        = height;
        info.depthStencilInfo.extent.depth         = 1;
        info.depthStencilInfo.format               = EPixelFormat::Depth32;
        info.depthStencilInfo.usage                = ETextureUsage::DepthStencilAttachment | ETextureUsage::Staging;
        info.depthStencilInfo.isDepthStencilBuffer = true;
        info.depthStencilInfo.isCompressed         = false;

        _renderTargets[i].Create(info);
    }

    onInitialize();
}

Window::~Window()
{
    Shutdown();
}

void Window::Shutdown()
{
    if (_isClosed)
    {
        return;
    }

    onShutdown();

    DestroyNativeWindow(_nativeWindow);

    for (auto& rt : _renderTargets)
    {
        rt.Clear();
    }

    _isClosed = true;
}

void Window::ProcessEvent()
{
    if (_shouldClose) // SDL Event 처리가 ImGui와 연동되지 않아 여기서 이중 처리
    {
        Flush();
        return;
    }

    NativeEvent event;
    while (PeekNativeEvent(&_nativeWindow, event))
    {
        event = PopNativeEvent(&_nativeWindow);
        switch (event.type)
        {
        case NativeEvent::Type::WindowOpen:
        {
            _shouldClose   = false;
            _shouldPresent = true;
            _shouldUpdate  = true;

            break;
        }
        case NativeEvent::Type::WindowClose:
        {
            _shouldClose   = true;
            _shouldUpdate  = false;
            _shouldPresent = false;

            break;
        }
        case NativeEvent::Type::WindowMaximize:
        {
            _shouldUpdate  = true;
            _shouldPresent = true;
            onSuspend();
            onRestore();

            break;
        }
        case NativeEvent::Type::WindowMinimize:
        {
            _shouldUpdate  = false;
            _shouldPresent = false;

            break;
        }
        case NativeEvent::Type::WindowResize:
        {
            onSuspend();
            onRestore();
            break;
        }
        case NativeEvent::Type::WindowMoveEnter:
        {
            _shouldUpdate  = false;
            _shouldPresent = false;
            onSuspend();

            break;
        }
        case NativeEvent::Type::WindowMoveExit:
        case NativeEvent::Type::WindowRestore:
        {
            _shouldUpdate  = true;
            _shouldPresent = true;
            onRestore();

            break;
        }
        case NativeEvent::Type::WindowMove:
        {

            break;
        }
        case NativeEvent::Type::WindowFocusIn:
        {
            _shouldUpdate  = true;
            _shouldPresent = true;
            break;
        }
        case NativeEvent::Type::WindowFocusOut:
        {
            _shouldUpdate  = false;
            _shouldPresent = false;
            break;
        }
        default:
            break;
        }
    }

    if (_shouldClose)
    {
        Flush();
        return;
    }

    for (auto* child : _childs)
    {
        child->ProcessEvent();
    }
}

void Window::NextFrame()
{
    onNextFrame();
}

void Window::Update(float deltaTime)
{
    onUpdate(deltaTime);
}

void Window::Render()
{
    onRender();
}

void Window::Present()
{
    onPresent();
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

void Window::onSuspend()
{
    _rhiContext->Suspend(_swapchain);
}

void Window::onRestore()
{
    _rhiContext->Restore(_swapchain);
}

Application* Window::GetApplication()
{
    return _ownerApp;
}

void Window::SetPreEventHandler(void* handler)
{
    _preEventHandler = handler;
    SetNativePreEventHandler(handler);
}

HS_NS_END
