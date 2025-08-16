#include "HAL/NativeWindow.h"

#include <queue>
#include <cassert>

HS_NS_BEGIN

static std::unordered_map<const NativeWindow*, std::queue<EWindowEvent>> s_eventQueueTable;

bool PeekNativeEvent(NativeWindow* pWindow, EWindowEvent& outEvent)
{
    if (pWindow == nullptr || s_eventQueueTable.find(pWindow) == s_eventQueueTable.end())
    {
        return false;
    }

    PollNativeEvent(*pWindow);

    outEvent = EWindowEvent::NONE;

    auto& eventQueue = s_eventQueueTable[pWindow];
    if (eventQueue.empty())
    {
        return false;
    }

    outEvent = eventQueue.front();

    return true;
}

void PushNativeEvent(const NativeWindow* pWindow, EWindowEvent event)
{
    assert(pWindow && s_eventQueueTable.find(pWindow) != s_eventQueueTable.end());

    auto& eventQueue = s_eventQueueTable[pWindow];

    eventQueue.push(event);
}

EWindowEvent PopNativeEvent(const NativeWindow* pWindow)
{
    assert(pWindow && s_eventQueueTable.find(pWindow) != s_eventQueueTable.end());

    auto& eventQueue = s_eventQueueTable[pWindow];

    EWindowEvent event = eventQueue.front();

    eventQueue.pop();

    return event;
}

HS_NS_END