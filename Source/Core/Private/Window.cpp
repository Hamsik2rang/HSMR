#include "Core/Window.h"

#include "Core/Log.h"

#include <queue>

HS_NS_BEGIN

std::unordered_map<const NativeWindow*, std::queue<EWindowEvent>> s_eventQueueTable;

Window::Window(const char* name, uint16 width, uint16 height, EWindowFlags flags)
    : _isClosed(false)
    , _shouldClose(false)
{
    if (!CreateNativeWindow(name, width, height, flags, _nativeWindow))
    {
        HS_LOG(crash, "Fail to create NativeWindow");
    }

    s_eventQueueTable.insert({&_nativeWindow, std::queue<EWindowEvent>()});

    onInitialize();
}

Window::~Window()
{
    Shutdown();
}
//
//void Window::NextFrame()
//{
//    onNextFrame();
//
//    for (auto* child : _childs)
//    {
//        child->NextFrame();
//    }
//}
//
//void Window::Update()
//{
//    onUpdate();
//
//    for (auto* child : _childs)
//    {
//        child->Update();
//    }
//}
//
//// Render는 Preorder로 수행
//void Window::Render()
//{
//    if (nullptr != _renderer)
//    {
//        onRender();
//    }
//
//    for (auto* child : _childs)
//    {
//        child->Render();
//    }
//}
//
//void Window::Present()
//{
//    onPresent();
//
//    for (auto* child : _childs)
//    {
//        child->Present();
//    }
//}

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

bool PeekNativeEvent(NativeWindow* pWindow, EWindowEvent& outEvent, bool remove)
{
    HS_ASSERT(s_eventQueueTable.find(pWindow) != s_eventQueueTable.end(), "NativeWindow is not created. you should call \'hs_platform_create_window()\' first.");

    PollNativeEvent(*pWindow);
    
    outEvent = EWindowEvent::NONE;

    auto& eventQueue = s_eventQueueTable[pWindow];
    if (eventQueue.empty())
    {
        return false;
    }

    outEvent = eventQueue.front();

    if (remove)
    {
        eventQueue.pop();
    }

    return true;
}

void PushNativeEvent(const NativeWindow* pWindow, EWindowEvent event)
{
    HS_ASSERT(s_eventQueueTable.find(pWindow) != s_eventQueueTable.end(), "NativeWindow is not created. you should call \'hs_platform_create_window()\' first.");

    auto& eventQueue = s_eventQueueTable[pWindow];

    eventQueue.push(event);
}

EWindowEvent PopNativeEvent(const NativeWindow* pWindow)
{
    HS_ASSERT(s_eventQueueTable.find(pWindow) != s_eventQueueTable.end(), "NativeWindow is not created. you should call \'hs_platform_create_window()\' first.");

    auto& eventQueue = s_eventQueueTable[pWindow];

    EWindowEvent event = eventQueue.front();

    eventQueue.pop();

    return event;
}
