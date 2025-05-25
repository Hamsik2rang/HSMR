#include "Engine/Platform/Mac/PlatformWindowMac.h"

#include "Engine/Core/EngineContext.h"
#include "Engine/Core/Window.h"

#include <cstring>
#include <unordered_map>
#include <queue>
#include <utility>

#import <MetalKit/MetalKit.h>
#import <QuartzCore/CAMetalLayer.h>

@implementation HSViewController
{
    CGSize _curDrawableSize;
    HS::NativeWindow* _pHsNativeWindow;
}
- (void)loadView
{
    NSRect frame = NSMakeRect(0, 0, _window.frame.size.width, _window.frame.size.height);
    self.view    = [[NSView alloc] initWithFrame:frame];

    [self.view setWantsLayer:YES];

    CAMetalLayer* layer   = [CAMetalLayer new];
    layer.device          = MTLCreateSystemDefaultDevice();
    layer.pixelFormat     = MTLPixelFormatBGRA8Unorm;
    layer.colorspace      = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    layer.framebufferOnly = YES;
    layer.drawableSize    = self.view.bounds.size;
    layer.contentsScale   = [[NSScreen mainScreen] backingScaleFactor];

    self.view.layer = layer;

    [self resizeDrawable];
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    [_window setContentView:self.view];
    [_window makeFirstResponder:self.view];

    [NSApp activateIgnoringOtherApps:YES];
}

- (instancetype)initWithWindow:(NSWindow*)window
{
    self = [super init];
    if (nil != self)
    {
        _window = window;
        [_window setDelegate:self];
    }

    return self;
}

- (void)viewWillAppear
{
}

- (void)windowDidResize:(NSNotification*)notification
{
    NSRect frame = [_window frame];
    [self.view setFrameSize:frame.size];
    [self resizeDrawable];

    NSLog(@"frameSize : %f %f", frame.size.width, frame.size.height);
}

- (void)
    windowDidBecomeMain:(NSNotification*)notification
{
    // 윈도우가 메인 윈도우가 되었을 때
    NSLog(@"Window became main");
}

- (void)windowDidBecomeKey:(NSNotification*)notification
{
    // 윈도우가 키 윈도우가 되었을 때
    NSLog(@"Window became key");
}

- (void)windowDidExpose:(NSNotification*)notification
{
    // 윈도우가 처음 화면에 표시될 때
    NSLog(@"Window did expose");
}

- (void)windowWillClose:(NSNotification*)notification
{
    hs_window_push_event(_pHsNativeWindow, HS::EWindowEvent::CLOSE);

    [_window setDelegate:nil];
    [_window setContentViewController:nil];

    _window = nil;
}

- (BOOL)windowShouldClose:(NSWindow*)sender
{
    NSLog(@"Window should close");

    return YES; // 윈도우 닫기 허용
}

- (CGSize)getBackingViewSize
{
    return _curDrawableSize;
}

- (void)resizeDrawable
{
    NSRect frameRect        = [_window frame];
    NSRect backingFrameRect = [_window convertRectToBacking:frameRect];

    CAMetalLayer* layer        = (CAMetalLayer*)self.view.layer;
    CGSize backingDrawableSize = CGSizeMake(backingFrameRect.size.width, backingFrameRect.size.height);
    _curDrawableSize           = backingDrawableSize;
    [layer setDrawableSize:_curDrawableSize];
}

- (void)setHSWindow:(HS::NativeWindow*)pHsNativeWindow
{
    _pHsNativeWindow = pHsNativeWindow;
}

@end

HS_NS_BEGIN

bool hs_platform_window_create(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow)
{
    NSRect frame     = NSMakeRect(0, 0, (CGFloat)width, (CGFloat)height);
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable backing:NSBackingStoreBuffered defer:NO];

    [window setTitle:@"HSMR"];

    HSViewController* vc = [[HSViewController alloc] initWithWindow:window];
    [window setContentViewController:vc];

    outNativeWindow.handle = (__bridge void*)window;

    NSScreen* mainScreen = [NSScreen mainScreen];
    NSRect screenRect    = mainScreen.frame;

    outNativeWindow.maxWidth  = static_cast<uint32>(screenRect.size.width);
    outNativeWindow.maxHeight = static_cast<uint32>(screenRect.size.height);
    outNativeWindow.scale     = static_cast<float>(mainScreen.backingScaleFactor);
    outNativeWindow.title     = name;
    outNativeWindow.width     = width;
    outNativeWindow.height    = height;
    outNativeWindow.minWidth  = 1;
    outNativeWindow.minHeight = 1;

    [vc setHSWindow:&outNativeWindow];
}

void hs_platform_window_destroy(NativeWindow& window)
{
    // Autorelease되므로 직접 Release할 필요 없음.
    //    NSWindow* handle = (__bridge NSWindow*)window.handle;
    //    [handle release];
}

void hs_platform_window_show(const NativeWindow& nativeWindow)
{
    NSWindow* window = (__bridge NSWindow*)(nativeWindow.handle);
    [window center];
    [window makeKeyAndOrderFront:nil];
    [window setIsVisible:YES];
    [window makeMainWindow];
    [window becomeKeyWindow];
}

void hs_platform_window_poll_event()
{
    @autoreleasepool
    {
        while (true)
        {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event == nil)
            {
                break;
            }

            [NSApp sendEvent:event];
        }
    }
}

HS_NS_END
