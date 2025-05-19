#include "Engine/Platform/Mac/PlatformWindowMac.h"

#include "Engine/Core/EngineContext.h"

#include <cstring>
#include <unordered_map>
#include <queue>

@implementation HSViewController

- (void)loadView
{
    NSRect frame = [_window frame];
    self.view    = [[MTKView alloc] initWithFrame:frame device:MTLCreateSystemDefaultDevice()];

    _mtkView = (MTKView*)self.view;
    if (nil == _mtkView.device)
    {
        HS_LOG(crash, "MTLDevice is not created!");
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.view.window.delegate = self;

    // 윈도우 표시 - 이 부분이 누락되었을 수 있음
    [_window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

- (instancetype)initWithWindow:(NSWindow*)window
{
    self = [super init];
    if (nil != self)
    {
        _window = window;
        [_window setContentViewController:self];
    }

    return self;
}

- (void)viewWillAppear
{
    auto bp = true;
}

- (void)windowDidBecomeMain:(NSNotification *)notification
{
    // 윈도우가 메인 윈도우가 되었을 때
    NSLog(@"Window became main");
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    // 윈도우가 키 윈도우가 되었을 때
    NSLog(@"Window became key");
}

- (void)windowDidExpose:(NSNotification *)notification
{
    // 윈도우가 처음 화면에 표시될 때
    NSLog(@"Window did expose");
}


- (void)windowWillClose:(NSNotification*)notification
{
}

@end

HS_NS_BEGIN

std::unordered_map<const NativeWindow*, std::queue<EWindowEvent>> g_eventQueueTable;

bool hs_platform_window_create(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow)
{
    outNativeWindow.title     = name;
    outNativeWindow.width     = width;
    outNativeWindow.height    = height;
    outNativeWindow.minWidth  = 1;
    outNativeWindow.minHeight = 1;

    NSRect    frame  = NSMakeRect(0, 0, (CGFloat)width, (CGFloat)height);
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable backing:NSBackingStoreBuffered defer:NO];

    [window setTitle:@"HSMR"];
    [window center];
    [window makeKeyAndOrderFront:nil];

    [window setContentViewController:[[HSViewController alloc] initWithWindow:window]];

    outNativeWindow.handle = (__bridge_retained void*)window;

    NSScreen* mainScreen = [NSScreen mainScreen];
    NSRect    screenRect = mainScreen.frame;

    outNativeWindow.maxWidth  = static_cast<uint32>(screenRect.size.width);
    outNativeWindow.maxHeight = static_cast<uint32>(screenRect.size.height);

    outNativeWindow.scale = static_cast<float>(mainScreen.backingScaleFactor);
}

void hs_platform_window_destroy(NativeWindow& window)
{
    NSWindow* handle = (__bridge_transfer NSWindow*)window.handle;

    while (false == g_eventQueueTable[&window].empty())
    {
        g_eventQueueTable[&window].pop();
    }
    g_eventQueueTable.erase(&window);

#ifndef HS_OBJC_ARC
    [handle dealloc];
#endif
}

void hs_platform_window_show()
{
}

bool hs_platform_window_peek_event(const NativeWindow* pWindow, EWindowEvent& outEvent)
{
    HS_ASSERT(g_eventQueueTable.find(pWindow) != g_eventQueueTable.end(), "NativeWindow is not created. you should call \'hs_platform_create_window()\' first.");

    outEvent = EWindowEvent::NONE;

    auto& eventQueue = g_eventQueueTable[pWindow];
    if (eventQueue.empty())
    {
        return false;
    }

    outEvent = eventQueue.front();
    return true;
}

void hs_platform_window_push_event(const NativeWindow* pWindow, EWindowEvent event)
{
    HS_ASSERT(g_eventQueueTable.find(pWindow) != g_eventQueueTable.end(), "NativeWindow is not created. you should call \'hs_platform_create_window()\' first.");

    auto& eventQueue = g_eventQueueTable[pWindow];

    eventQueue.push(event);
}

EWindowEvent hs_platform_window_pop_event(const NativeWindow* pWindow)
{
    HS_ASSERT(g_eventQueueTable.find(pWindow) != g_eventQueueTable.end(), "NativeWindow is not created. you should call \'hs_platform_create_window()\' first.");

    auto& eventQueue = g_eventQueueTable[pWindow];

    EWindowEvent event = eventQueue.front();

    eventQueue.pop();

    return event;
}

HS_NS_END
