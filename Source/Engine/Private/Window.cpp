#include "Engine/Window.h"

#include "Core/Log.h"

#include <queue>

HS_NS_BEGIN


Window::Window(const char* name, uint16 width, uint16 height, EWindowFlags flags)
    : _isClosed(false)
    , _shouldClose(false)
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

using namespace HS;
