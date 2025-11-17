#include "Platform/NativeWindow.h"

#if defined(__WINDOWS__)
#include "Platform/Win/WinWindow.h"
#elif defined(__APPLE__)
//#include "Platform/Mac/MacWindow.h"
#endif

#include <queue>
#include <cassert>

HS_NS_BEGIN

bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow);
void DestroyNativeWindowInternal(NativeWindow& nativeWindow);
void ShowNativeWindowInternal(const NativeWindow& nativeWindow);
void PollNativeEventInternal(NativeWindow& nativeWindow);
void SetNativeWindowSizeInternal(uint16 width, uint16 height);
void GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight);

static std::unordered_map<const NativeWindow*, std::queue<NativeEvent>> s_eventQueueTable;

bool CreateNativeWindow(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow) 
{
	bool result = CreateNativeWindowInternal(name, width, height, flag, outNativeWindow);

	assert(s_eventQueueTable.find(&outNativeWindow) == s_eventQueueTable.end());

	s_eventQueueTable.insert(std::make_pair(&outNativeWindow, std::queue<NativeEvent>()));

	return result;
}

void DestroyNativeWindow(NativeWindow& nativeWindow) 
{
	DestroyNativeWindowInternal(nativeWindow);
}

void ShowNativeWindow(const NativeWindow& nativeWindow)
{
	ShowNativeWindowInternal(nativeWindow);
}

void PollNativeEvent(NativeWindow& nativeWindow) 
{
	PollNativeEventInternal(nativeWindow);
}

void SetNativeWindowSize(uint16 width, uint16 height) 
{
	SetNativeWindowSizeInternal(width, height);
}

void GetNativeWindowSize(uint16& outWidth, uint16& outHeight) 
{
	GetNativeWindowSizeInternal(outWidth, outHeight);
}

bool PeekNativeEvent(NativeWindow* pWindow, NativeEvent& outEvent)
{
	if (pWindow == nullptr || s_eventQueueTable.find(pWindow) == s_eventQueueTable.end())
	{
		return false;
	}

	PollNativeEvent(*pWindow);

	outEvent = NativeEvent::Type::NONE;

	auto& eventQueue = s_eventQueueTable[pWindow];
	if (eventQueue.empty())
	{
		return false;
	}

	outEvent = eventQueue.front();

	return true;
}

void PushNativeEvent(const NativeWindow* pWindow, NativeEvent event)
{
	assert(pWindow && s_eventQueueTable.find(pWindow) != s_eventQueueTable.end());

	auto& eventQueue = s_eventQueueTable[pWindow];

	eventQueue.push(event);
}

NativeEvent PopNativeEvent(const NativeWindow* pWindow)
{
	assert(pWindow && s_eventQueueTable.find(pWindow) != s_eventQueueTable.end());

	auto& eventQueue = s_eventQueueTable[pWindow];

	NativeEvent event = eventQueue.front();

	eventQueue.pop();

	return event;
}

HS_NS_END
