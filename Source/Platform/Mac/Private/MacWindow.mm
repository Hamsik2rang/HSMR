#include "Platform/Mac/MacWindow.h"

#include <cstring>
#include <unordered_map>
#include <queue>
#include <utility>

#include "Core/HAL/Input.h"

namespace {
// macOS keycode to Input::Button mapping
hs::Input::Button MacKeyCodeToButton(unsigned short keyCode)
{
    switch (keyCode)
    {
        // Letters
        case 0x00: return hs::Input::Button::A;
        case 0x0B: return hs::Input::Button::B;
        case 0x08: return hs::Input::Button::C;
        case 0x02: return hs::Input::Button::D;
        case 0x0E: return hs::Input::Button::E;
        case 0x03: return hs::Input::Button::F;
        case 0x05: return hs::Input::Button::G;
        case 0x04: return hs::Input::Button::H;
        case 0x22: return hs::Input::Button::I;
        case 0x26: return hs::Input::Button::J;
        case 0x28: return hs::Input::Button::K;
        case 0x25: return hs::Input::Button::L;
        case 0x2E: return hs::Input::Button::M;
        case 0x2D: return hs::Input::Button::N;
        case 0x1F: return hs::Input::Button::O;
        case 0x23: return hs::Input::Button::P;
        case 0x0C: return hs::Input::Button::Q;
        case 0x0F: return hs::Input::Button::R;
        case 0x01: return hs::Input::Button::S;
        case 0x11: return hs::Input::Button::T;
        case 0x20: return hs::Input::Button::U;
        case 0x09: return hs::Input::Button::V;
        case 0x0D: return hs::Input::Button::W;
        case 0x07: return hs::Input::Button::X;
        case 0x10: return hs::Input::Button::Y;
        case 0x06: return hs::Input::Button::Z;

        // Numbers
        case 0x1D: return hs::Input::Button::NUM_0;
        case 0x12: return hs::Input::Button::NUM_1;
        case 0x13: return hs::Input::Button::NUM_2;
        case 0x14: return hs::Input::Button::NUM_3;
        case 0x15: return hs::Input::Button::NUM_4;
        case 0x17: return hs::Input::Button::NUM_5;
        case 0x16: return hs::Input::Button::NUM_6;
        case 0x1A: return hs::Input::Button::NUM_7;
        case 0x1C: return hs::Input::Button::NUM_8;
        case 0x19: return hs::Input::Button::NUM_9;

        // Function keys
        case 0x7A: return hs::Input::Button::F1;
        case 0x78: return hs::Input::Button::F2;
        case 0x63: return hs::Input::Button::F3;
        case 0x76: return hs::Input::Button::F4;
        case 0x60: return hs::Input::Button::F5;
        case 0x61: return hs::Input::Button::F6;
        case 0x62: return hs::Input::Button::F7;
        case 0x64: return hs::Input::Button::F8;
        case 0x65: return hs::Input::Button::F9;
        case 0x6D: return hs::Input::Button::F10;
        case 0x67: return hs::Input::Button::F11;
        case 0x6F: return hs::Input::Button::F12;

        // Modifiers & special keys
        case 0x38: return hs::Input::Button::SHIFT;     // Left Shift
        case 0x3C: return hs::Input::Button::SHIFT;     // Right Shift
        case 0x3B: return hs::Input::Button::CONTROL;   // Left Control
        case 0x3E: return hs::Input::Button::CONTROL;   // Right Control
        case 0x3A: return hs::Input::Button::ALT;       // Left Option
        case 0x3D: return hs::Input::Button::ALT;       // Right Option
        case 0x37: return hs::Input::Button::LWIN_OR_COMMAND; // Left Command
        case 0x36: return hs::Input::Button::LWIN_OR_COMMAND; // Right Command

        case 0x31: return hs::Input::Button::SPACE;
        case 0x30: return hs::Input::Button::TAB;
        case 0x33: return hs::Input::Button::BACK;      // Delete (Backspace)
        case 0x75: return hs::Input::Button::DELETE;    // Forward Delete
        case 0x24: return hs::Input::Button::UNKNOWN;   // Return (not mapped)
        case 0x35: return hs::Input::Button::UNKNOWN;   // Escape (not mapped)

        // Arrow keys
        case 0x7B: return hs::Input::Button::LEFT;
        case 0x7C: return hs::Input::Button::RIGHT;
        case 0x7D: return hs::Input::Button::DOWN;
        case 0x7E: return hs::Input::Button::UP;

        // Navigation keys
        case 0x73: return hs::Input::Button::HOME;
        case 0x77: return hs::Input::Button::END;

        // Numpad
        case 0x52: return hs::Input::Button::NUMPAD_0;
        case 0x53: return hs::Input::Button::NUMPAD_1;
        case 0x54: return hs::Input::Button::NUMPAD_2;
        case 0x55: return hs::Input::Button::NUMPAD_3;
        case 0x56: return hs::Input::Button::NUMPAD_4;
        case 0x57: return hs::Input::Button::NUMPAD_5;
        case 0x58: return hs::Input::Button::NUMPAD_6;
        case 0x59: return hs::Input::Button::NUMPAD_7;
        case 0x5B: return hs::Input::Button::NUMPAD_8;
        case 0x5C: return hs::Input::Button::NUMPAD_9;
        case 0x43: return hs::Input::Button::MULTIPLY;
        case 0x45: return hs::Input::Button::ADD;
        case 0x4E: return hs::Input::Button::SUBTRACT;
        case 0x41: return hs::Input::Button::DECIMAL;
        case 0x4B: return hs::Input::Button::DIVIDE;

        default: return hs::Input::Button::UNKNOWN;
    }
}

void ProcessKeyEvent(NSEvent* event, bool isDown)
{
    hs::Input::Button button = MacKeyCodeToButton([event keyCode]);
    if (button != hs::Input::Button::UNKNOWN)
    {
        uint8 index = static_cast<uint8>(button);
        hs::Input::s_button[index].isPressed = isDown ? 1 : 0;
        hs::Input::s_button[index].repeatCount = isDown ? ([event isARepeat] ? 1 : 0) : 0;
    }
}

void ProcessModifierFlags(NSEventModifierFlags flags)
{
    // Shift
    hs::Input::s_button[static_cast<uint8>(hs::Input::Button::SHIFT)].isPressed =
        (flags & NSEventModifierFlagShift) ? 1 : 0;

    // Control
    hs::Input::s_button[static_cast<uint8>(hs::Input::Button::CONTROL)].isPressed =
        (flags & NSEventModifierFlagControl) ? 1 : 0;

    // Option (Alt)
    hs::Input::s_button[static_cast<uint8>(hs::Input::Button::ALT)].isPressed =
        (flags & NSEventModifierFlagOption) ? 1 : 0;

    // Command
    hs::Input::s_button[static_cast<uint8>(hs::Input::Button::LWIN_OR_COMMAND)].isPressed =
        (flags & NSEventModifierFlagCommand) ? 1 : 0;
}

void ProcessMouseButtonEvent(NSEvent* event, bool isDown)
{
    hs::Input::Button button = hs::Input::Button::UNKNOWN;

    switch ([event type])
    {
        case NSEventTypeLeftMouseDown:
        case NSEventTypeLeftMouseUp:
            button = hs::Input::Button::MOUSE_LEFT;
            break;
        case NSEventTypeRightMouseDown:
        case NSEventTypeRightMouseUp:
            button = hs::Input::Button::MOUSE_RIGHT;
            break;
        case NSEventTypeOtherMouseDown:
        case NSEventTypeOtherMouseUp:
            button = hs::Input::Button::MOUSE_MIDDLE;
            break;
        default:
            break;
    }

    if (button != hs::Input::Button::UNKNOWN)
    {
        hs::Input::s_button[static_cast<uint8>(button)].isPressed = isDown ? 1 : 0;
    }
}

void ProcessMouseMoveEvent(NSEvent* event)
{
    NSPoint location = [event locationInWindow];

    hs::Input::s_move.xPoint = static_cast<uint16>(location.x);
    hs::Input::s_move.yPoint = static_cast<uint16>(location.y);
    hs::Input::s_move.isMoved = 1;
}

void ProcessScrollEvent(NSEvent* event)
{
    CGFloat deltaX = [event scrollingDeltaX];
    CGFloat deltaY = [event scrollingDeltaY];

    hs::Input::s_scroll.hOffset = static_cast<uint16>(deltaX);
    hs::Input::s_scroll.vOffset = static_cast<uint16>(deltaY);
    hs::Input::s_scroll.isScrolled = 1;
}

} // anonymous namespace

@implementation HSViewController
{
    CGSize _curDrawableSize;
    hs::NativeWindow* _pHsNativeWindow;
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
    
    auto curDrawableSize = layer.drawableSize;
    
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
    [self resizeDrawable];
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
    PushNativeEvent(_pHsNativeWindow, hs::NativeEvent::Type::WINDOW_CLOSE);

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
    NSRect contentViewSize        = [_window contentView].frame;
    NSRect backingFrameRect = [_window convertRectToBacking:contentViewSize];

    CAMetalLayer* layer        = (CAMetalLayer*)self.view.layer;
    
    CGSize backingDrawableSize = CGSizeMake(backingFrameRect.size.width, backingFrameRect.size.height);
    _curDrawableSize           = backingDrawableSize;
    [layer setDrawableSize:_curDrawableSize];
}

- (void)setHSWindow:(hs::NativeWindow*)pHsNativeWindow
{
    _pHsNativeWindow = pHsNativeWindow;
}

@end

HS_NS_BEGIN

bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow)
{
    NSRect frame     = NSMakeRect(0, 0, (CGFloat)width, (CGFloat)height);
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable backing:NSBackingStoreBuffered defer:NO];

    [window setTitle:@"HSMR"];

    HSViewController* vc = [[HSViewController alloc] initWithWindow:window];
    [window setContentViewController:vc];

    outNativeWindow.handle = (__bridge void*)window;

    NSScreen* mainScreen = [NSScreen mainScreen];
    NSRect screenRect    = mainScreen.frame;

    outNativeWindow.scale     = static_cast<float>(mainScreen.backingScaleFactor);
    outNativeWindow.title     = name;
    outNativeWindow.width     = width;
    outNativeWindow.height    = height;
    
    outNativeWindow.surfaceWidth = width;
    outNativeWindow.surfaceHeight = height;

    [vc setHSWindow:&outNativeWindow];
    
    return true;
}

void DestroyNativeWindowInternal(NativeWindow& window)
{
    // Autorelease되므로 직접 Release할 필요 없음.
    //    NSWindow* handle = (__bridge NSWindow*)window.handle;
    //    [handle release];
}

void ShowNativeWindowInternal(const NativeWindow& nativeWindow)
{
    NSWindow* window = (__bridge NSWindow*)(nativeWindow.handle);
    [window center];
    [window makeKeyAndOrderFront:nil];
    [window setIsVisible:YES];
    [window makeMainWindow];
    [window becomeKeyWindow];
}

void PollNativeEventInternal(NativeWindow& window)
{
    // Reset per-frame input states
    Input::s_move.isMoved = 0;
    Input::s_scroll.isScrolled = 0;

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

            // Process input events
            switch ([event type])
            {
                case NSEventTypeKeyDown:
                    ProcessKeyEvent(event, true);
                    break;
                case NSEventTypeKeyUp:
                    ProcessKeyEvent(event, false);
                    break;
                case NSEventTypeFlagsChanged:
                    ProcessModifierFlags([event modifierFlags]);
                    break;
                case NSEventTypeLeftMouseDown:
                case NSEventTypeRightMouseDown:
                case NSEventTypeOtherMouseDown:
                    ProcessMouseButtonEvent(event, true);
                    break;
                case NSEventTypeLeftMouseUp:
                case NSEventTypeRightMouseUp:
                case NSEventTypeOtherMouseUp:
                    ProcessMouseButtonEvent(event, false);
                    break;
                case NSEventTypeMouseMoved:
                case NSEventTypeLeftMouseDragged:
                case NSEventTypeRightMouseDragged:
                case NSEventTypeOtherMouseDragged:
                    ProcessMouseMoveEvent(event);
                    break;
                case NSEventTypeScrollWheel:
                    ProcessScrollEvent(event);
                    break;
                default:
                    break;
            }

            [NSApp sendEvent:event];
        }
    }
}

void SetNativeWindowSizeInternal(uint16 width, uint16 height)
{
    
}

void GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight)
{
    
}

void SetNativePreEventHandler(void* fnHandler)
{
    // empty.
}

HS_NS_END
