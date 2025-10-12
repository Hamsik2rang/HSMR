#include "Engine/Window.h"
#include "Engine/Application.h"

#include "HAL/NativeWindow.h"

#include "Core/Log.h"

#include <queue>

HS_NS_BEGIN

Window::Window(Application* ownerApp, const char* name, uint16 width, uint16 height, EWindowFlags flags)
    : _isClosed(false)
    , _shouldClose(false)
    , _ownerApp(ownerApp)
    , _preEventHandler(nullptr)
{
    if (!CreateNativeWindow(name, width, height, flags, _nativeWindow))
    {
        HS_LOG(crash, "Fail to create NativeWindow");
    }

    onInitialize();
}

Window::~Window()
{
    Shutdown();
}

void Window::Shutdown()
{
    onShutdown();

    DestroyNativeWindow(_nativeWindow);

    _isClosed = true;
}

void Window::ProcessEvent()
{
    NativeEvent event;
    while (PeekNativeEvent(&_nativeWindow, event))
    {
        event = PopNativeEvent(&_nativeWindow);
        switch (event.type)
        {
        case NativeEvent::Type::WINDOW_OPEN:
        {
            _shouldClose = false;

            break;
        }
        case NativeEvent::Type::WINDOW_CLOSE:
        {
            _shouldClose   = true;
            _shouldUpdate  = false;
            _shouldPresent = false;

            break;
        }
        case NativeEvent::Type::WINDOW_MAXIMIZE:
        {
            _shouldUpdate  = true;
            _shouldPresent = true;
            onRestore();

            break;
        }
        case NativeEvent::Type::WINDOW_MINIMIZE:
        {
            _shouldUpdate  = false;
            _shouldPresent = false;

            break;
        }
        case NativeEvent::Type::WINDOW_RESIZE:
        {
            onSuspend();
            onRestore();
            break;
        }
        case NativeEvent::Type::WINDOW_MOVE_ENTER:
        {
            _shouldUpdate  = false;
            _shouldPresent = false;
            onSuspend();

            break;
        }
        case NativeEvent::Type::WINDOW_MOVE_EXIT:
        case NativeEvent::Type::WINDOW_RESTORE:
        {
            _shouldUpdate  = true;
            _shouldPresent = true;
            onRestore();

            break;
        }
        case NativeEvent::Type::WINDOW_MOVE:
        {

            break;
        }
        case NativeEvent::Type::WINDOW_FOCUS_IN:
        {

            break;
        }
        case NativeEvent::Type::WINDOW_FOCUS_OUT:
        {

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

void Window::Update()
{
    onUpdate();
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

using namespace HS;
