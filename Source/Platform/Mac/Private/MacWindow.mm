//
//  MacWindow.mm
//  Platform
//
//  Created by Yongsik Im on 5/16/2025
//

#include "Platform/Mac/MacWindow.h"

#include <cstring>
#include <unordered_map>
#include <queue>
#include <utility>

#include "Core/HAL/Input.h"
#include "Core/Log.h"

#ifdef __SDL__
#include <SDL3/SDL.h>
#include <SDL3/SDL_metal.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

// SDL-specific static variables
static SDL_Window* s_sdlWindow           = nullptr;
static SDL_MetalView s_metalView         = nullptr;
static hs::NativeWindow* s_boundHsWindow = nullptr;

// Pre-event handler for ImGui integration
static bool (*s_preEventHandler)(SDL_Event*) = nullptr;

namespace
{
// Map SDL scancode to Input::Button enum value
static uint8 MapSDLScancodeToButton(SDL_Scancode scancode)
{
    using Button = hs::Input::Button;

    switch (scancode)
    {
    // Letters A-Z
    case SDL_SCANCODE_A:           return static_cast<uint8>(Button::A);
    case SDL_SCANCODE_B:           return static_cast<uint8>(Button::B);
    case SDL_SCANCODE_C:           return static_cast<uint8>(Button::C);
    case SDL_SCANCODE_D:           return static_cast<uint8>(Button::D);
    case SDL_SCANCODE_E:           return static_cast<uint8>(Button::E);
    case SDL_SCANCODE_F:           return static_cast<uint8>(Button::F);
    case SDL_SCANCODE_G:           return static_cast<uint8>(Button::G);
    case SDL_SCANCODE_H:           return static_cast<uint8>(Button::H);
    case SDL_SCANCODE_I:           return static_cast<uint8>(Button::I);
    case SDL_SCANCODE_J:           return static_cast<uint8>(Button::J);
    case SDL_SCANCODE_K:           return static_cast<uint8>(Button::K);
    case SDL_SCANCODE_L:           return static_cast<uint8>(Button::L);
    case SDL_SCANCODE_M:           return static_cast<uint8>(Button::M);
    case SDL_SCANCODE_N:           return static_cast<uint8>(Button::N);
    case SDL_SCANCODE_O:           return static_cast<uint8>(Button::O);
    case SDL_SCANCODE_P:           return static_cast<uint8>(Button::P);
    case SDL_SCANCODE_Q:           return static_cast<uint8>(Button::Q);
    case SDL_SCANCODE_R:           return static_cast<uint8>(Button::R);
    case SDL_SCANCODE_S:           return static_cast<uint8>(Button::S);
    case SDL_SCANCODE_T:           return static_cast<uint8>(Button::T);
    case SDL_SCANCODE_U:           return static_cast<uint8>(Button::U);
    case SDL_SCANCODE_V:           return static_cast<uint8>(Button::V);
    case SDL_SCANCODE_W:           return static_cast<uint8>(Button::W);
    case SDL_SCANCODE_X:           return static_cast<uint8>(Button::X);
    case SDL_SCANCODE_Y:           return static_cast<uint8>(Button::Y);
    case SDL_SCANCODE_Z:           return static_cast<uint8>(Button::Z);

    // Numbers 0-9
    case SDL_SCANCODE_0:           return static_cast<uint8>(Button::NUM_0);
    case SDL_SCANCODE_1:           return static_cast<uint8>(Button::NUM_1);
    case SDL_SCANCODE_2:           return static_cast<uint8>(Button::NUM_2);
    case SDL_SCANCODE_3:           return static_cast<uint8>(Button::NUM_3);
    case SDL_SCANCODE_4:           return static_cast<uint8>(Button::NUM_4);
    case SDL_SCANCODE_5:           return static_cast<uint8>(Button::NUM_5);
    case SDL_SCANCODE_6:           return static_cast<uint8>(Button::NUM_6);
    case SDL_SCANCODE_7:           return static_cast<uint8>(Button::NUM_7);
    case SDL_SCANCODE_8:           return static_cast<uint8>(Button::NUM_8);
    case SDL_SCANCODE_9:           return static_cast<uint8>(Button::NUM_9);

    // Function keys
    case SDL_SCANCODE_F1:          return static_cast<uint8>(Button::F1);
    case SDL_SCANCODE_F2:          return static_cast<uint8>(Button::F2);
    case SDL_SCANCODE_F3:          return static_cast<uint8>(Button::F3);
    case SDL_SCANCODE_F4:          return static_cast<uint8>(Button::F4);
    case SDL_SCANCODE_F5:          return static_cast<uint8>(Button::F5);
    case SDL_SCANCODE_F6:          return static_cast<uint8>(Button::F6);
    case SDL_SCANCODE_F7:          return static_cast<uint8>(Button::F7);
    case SDL_SCANCODE_F8:          return static_cast<uint8>(Button::F8);
    case SDL_SCANCODE_F9:          return static_cast<uint8>(Button::F9);
    case SDL_SCANCODE_F10:         return static_cast<uint8>(Button::F10);
    case SDL_SCANCODE_F11:         return static_cast<uint8>(Button::F11);
    case SDL_SCANCODE_F12:         return static_cast<uint8>(Button::F12);

    // Special keys
    case SDL_SCANCODE_BACKSPACE:   return static_cast<uint8>(Button::BACK);
    case SDL_SCANCODE_TAB:         return static_cast<uint8>(Button::TAB);
    case SDL_SCANCODE_LSHIFT:
    case SDL_SCANCODE_RSHIFT:      return static_cast<uint8>(Button::SHIFT);
    case SDL_SCANCODE_LCTRL:
    case SDL_SCANCODE_RCTRL:       return static_cast<uint8>(Button::CONTROL);
    case SDL_SCANCODE_LALT:
    case SDL_SCANCODE_RALT:        return static_cast<uint8>(Button::ALT);
    case SDL_SCANCODE_SPACE:       return static_cast<uint8>(Button::SPACE);
    case SDL_SCANCODE_END:         return static_cast<uint8>(Button::END);
    case SDL_SCANCODE_HOME:        return static_cast<uint8>(Button::HOME);
    case SDL_SCANCODE_LEFT:        return static_cast<uint8>(Button::LEFT);
    case SDL_SCANCODE_UP:          return static_cast<uint8>(Button::UP);
    case SDL_SCANCODE_RIGHT:       return static_cast<uint8>(Button::RIGHT);
    case SDL_SCANCODE_DOWN:        return static_cast<uint8>(Button::DOWN);
    case SDL_SCANCODE_INSERT:      return static_cast<uint8>(Button::INSERT);
    case SDL_SCANCODE_DELETE:      return static_cast<uint8>(Button::DELETE);

    // Numpad
    case SDL_SCANCODE_KP_0:        return static_cast<uint8>(Button::NUMPAD_0);
    case SDL_SCANCODE_KP_1:        return static_cast<uint8>(Button::NUMPAD_1);
    case SDL_SCANCODE_KP_2:        return static_cast<uint8>(Button::NUMPAD_2);
    case SDL_SCANCODE_KP_3:        return static_cast<uint8>(Button::NUMPAD_3);
    case SDL_SCANCODE_KP_4:        return static_cast<uint8>(Button::NUMPAD_4);
    case SDL_SCANCODE_KP_5:        return static_cast<uint8>(Button::NUMPAD_5);
    case SDL_SCANCODE_KP_6:        return static_cast<uint8>(Button::NUMPAD_6);
    case SDL_SCANCODE_KP_7:        return static_cast<uint8>(Button::NUMPAD_7);
    case SDL_SCANCODE_KP_8:        return static_cast<uint8>(Button::NUMPAD_8);
    case SDL_SCANCODE_KP_9:        return static_cast<uint8>(Button::NUMPAD_9);
    case SDL_SCANCODE_KP_MULTIPLY: return static_cast<uint8>(Button::MULTIPLY);
    case SDL_SCANCODE_KP_PLUS:     return static_cast<uint8>(Button::ADD);
    case SDL_SCANCODE_KP_MINUS:    return static_cast<uint8>(Button::SUBTRACT);
    case SDL_SCANCODE_KP_PERIOD:   return static_cast<uint8>(Button::DECIMAL);
    case SDL_SCANCODE_KP_DIVIDE:   return static_cast<uint8>(Button::DIVIDE);

    // Windows/Command key
    case SDL_SCANCODE_LGUI:        return static_cast<uint8>(Button::LWIN_OR_COMMAND);
    case SDL_SCANCODE_RGUI:        return static_cast<uint8>(Button::RWIN);

    default:                       return static_cast<uint8>(Button::UNKNOWN);
    }
}
} // anonymous namespace

#else // !__SDL__ - Native macOS path

namespace
{
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
    case 0x38: return hs::Input::Button::SHIFT;           // Left Shift
    case 0x3C: return hs::Input::Button::SHIFT;           // Right Shift
    case 0x3B: return hs::Input::Button::CONTROL;         // Left Control
    case 0x3E: return hs::Input::Button::CONTROL;         // Right Control
    case 0x3A: return hs::Input::Button::ALT;             // Left Option
    case 0x3D: return hs::Input::Button::ALT;             // Right Option
    case 0x37: return hs::Input::Button::LWIN_OR_COMMAND; // Left Command
    case 0x36: return hs::Input::Button::LWIN_OR_COMMAND; // Right Command

    case 0x31: return hs::Input::Button::SPACE;
    case 0x30: return hs::Input::Button::TAB;
    case 0x33: return hs::Input::Button::BACK;    // Delete (Backspace)
    case 0x75: return hs::Input::Button::DELETE;  // Forward Delete
    case 0x24: return hs::Input::Button::UNKNOWN; // Return (not mapped)
    case 0x35:
        return hs::Input::Button::UNKNOWN; // Escape (not mapped)

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

    default:   return hs::Input::Button::UNKNOWN;
    }
}

void ProcessKeyEvent(NSEvent* event, bool isDown)
{
    hs::Input::Button button = MacKeyCodeToButton([event keyCode]);
    if (button != hs::Input::Button::UNKNOWN)
    {
        uint8 index                            = static_cast<uint8>(button);
        hs::Input::s_button[index].isPressed   = isDown ? 1 : 0;
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

    hs::Input::s_move.xPoint  = static_cast<uint16>(location.x);
    hs::Input::s_move.yPoint  = static_cast<uint16>(location.y);
    hs::Input::s_move.isMoved = 1;
}

void ProcessScrollEvent(NSEvent* event)
{
    CGFloat deltaX = [event scrollingDeltaX];
    CGFloat deltaY = [event scrollingDeltaY];

    hs::Input::s_scroll.hOffset    = static_cast<uint16>(deltaX);
    hs::Input::s_scroll.vOffset    = static_cast<uint16>(deltaY);
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
    NSRect contentViewSize  = [_window contentView].frame;
    NSRect backingFrameRect = [_window convertRectToBacking:contentViewSize];

    CAMetalLayer* layer = (CAMetalLayer*)self.view.layer;

    CGSize backingDrawableSize = CGSizeMake(backingFrameRect.size.width, backingFrameRect.size.height);
    _curDrawableSize           = backingDrawableSize;
    [layer setDrawableSize:_curDrawableSize];
}

- (void)setHSWindow:(hs::NativeWindow*)pHsNativeWindow
{
    _pHsNativeWindow = pHsNativeWindow;
}

@end

#endif // !__SDL__

HS_NS_BEGIN

bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow)
{
#ifdef __SDL__
    // === SDL3 + Metal path ===

    // Convert EWindowFlags to SDL_WindowFlags and ensure Metal flag is set
    SDL_WindowFlags sdlFlags = static_cast<SDL_WindowFlags>(static_cast<uint64>(flag));
    sdlFlags |= SDL_WINDOW_METAL;

    // Create SDL window
    s_sdlWindow = SDL_CreateWindow(name, width, height, sdlFlags);
    if (!s_sdlWindow)
    {
        HS_LOG(crash, "Failed to create SDL window: %s", SDL_GetError());
        return false;
    }

    // Create Metal view from SDL window
    s_metalView = SDL_Metal_CreateView(s_sdlWindow);
    if (!s_metalView)
    {
        HS_LOG(crash, "Failed to create SDL Metal view: %s", SDL_GetError());
        SDL_DestroyWindow(s_sdlWindow);
        s_sdlWindow = nullptr;
        return false;
    }

    // Get the Metal layer and configure it
    CAMetalLayer* layer   = (__bridge CAMetalLayer*)SDL_Metal_GetLayer(s_metalView);
    layer.device          = MTLCreateSystemDefaultDevice();
    layer.pixelFormat     = MTLPixelFormatBGRA8Unorm;
    layer.framebufferOnly = YES;

    // Get actual drawable size (may differ on high-DPI displays)
    int drawableWidth, drawableHeight;
    SDL_GetWindowSizeInPixels(s_sdlWindow, &drawableWidth, &drawableHeight);

    float displayScale = SDL_GetWindowDisplayScale(s_sdlWindow);

    // Configure layer drawable size
    layer.drawableSize  = CGSizeMake(drawableWidth, drawableHeight);
    layer.contentsScale = displayScale;

    s_boundHsWindow = &outNativeWindow;

    // Populate NativeWindow struct
    outNativeWindow               = {};
    outNativeWindow.handle        = s_sdlWindow;
    outNativeWindow.graphicsView  = s_metalView;
    outNativeWindow.graphicsLayer = (__bridge void*)layer;
    outNativeWindow.width         = width;
    outNativeWindow.height        = height;
    outNativeWindow.surfaceWidth  = static_cast<uint16>(drawableWidth);
    outNativeWindow.surfaceHeight = static_cast<uint16>(drawableHeight);
    outNativeWindow.flags         = flag;
    outNativeWindow.title         = name;
    //    outNativeWindow.scale = displayScale;
    outNativeWindow.scale         = 1.0f;
    outNativeWindow.isMaximized   = (static_cast<uint64>(flag) & SDL_WINDOW_MAXIMIZED) != 0;
    outNativeWindow.isMinimized   = (static_cast<uint64>(flag) & SDL_WINDOW_MINIMIZED) != 0;
    outNativeWindow.resizable     = (flag & EWindowFlags::WINDOW_RESIZABLE) != EWindowFlags::NONE;
    outNativeWindow.useHDR        = (flag & EWindowFlags::WINDOW_HIGH_PIXEL_DENSITY) != EWindowFlags::NONE;

    HS_LOG(info, "[SDL] Window created: %s (%dx%d, surface: %dx%d, scale: %.2f)", name, width, height, drawableWidth, drawableHeight, displayScale);

#else
    // === Native NSWindow + HSViewController path ===

    NSRect frame     = NSMakeRect(0, 0, (CGFloat)width, (CGFloat)height);
    NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable backing:NSBackingStoreBuffered defer:NO];

    [window setTitle:@"HSMR"];

    HSViewController* vc = [[HSViewController alloc] initWithWindow:window];
    [window setContentViewController:vc];

    NSScreen* mainScreen = [NSScreen mainScreen];
    NSRect screenRect    = mainScreen.frame;

    // Populate NativeWindow struct
    outNativeWindow.handle        = (__bridge void*)window;
    outNativeWindow.graphicsView  = (__bridge void*)[vc view];
    outNativeWindow.graphicsLayer = (__bridge void*)[[vc view] layer];
    //    outNativeWindow.scale     = static_cast<float>(mainScreen.backingScaleFactor);
    outNativeWindow.scale         = 1.0f;
    outNativeWindow.title         = name;
    outNativeWindow.width         = width;
    outNativeWindow.height        = height;
    outNativeWindow.surfaceWidth  = width;
    outNativeWindow.surfaceHeight = height;

    [vc setHSWindow:&outNativeWindow];
#endif

    return true;
}

void DestroyNativeWindowInternal(NativeWindow& nativeWindow)
{
#ifdef __SDL__
    if (s_metalView)
    {
        SDL_Metal_DestroyView(s_metalView);
        s_metalView = nullptr;
    }

    if (s_sdlWindow)
    {
        SDL_DestroyWindow(s_sdlWindow);
        s_sdlWindow = nullptr;
    }

    s_boundHsWindow            = nullptr;
    nativeWindow.graphicsView  = nullptr;
    nativeWindow.graphicsLayer = nullptr;
#else
    // Autorelease되므로 직접 Release할 필요 없음.
    //    NSWindow* handle = (__bridge NSWindow*)window.handle;
    //    [handle release];
#endif
}

void ShowNativeWindowInternal(const NativeWindow& nativeWindow)
{
#ifdef __SDL__
    if (s_sdlWindow)
    {
        SDL_ShowWindow(s_sdlWindow);
    }
#else
    NSWindow* window = (__bridge NSWindow*)(nativeWindow.handle);
    [window center];
    [window makeKeyAndOrderFront:nil];
    [window setIsVisible:YES];
    [window makeMainWindow];
    [window becomeKeyWindow];
#endif
}

void PollNativeEventInternal(NativeWindow& nativeWindow)
{
    // Reset per-frame input states
    Input::s_move.isMoved      = 0;
    Input::s_scroll.isScrolled = 0;

#ifdef __SDL__
    s_boundHsWindow = &nativeWindow;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // Let ImGui process the event first
        if (s_preEventHandler)
        {
            s_preEventHandler(&event);
        }

        // Process window and input events
        switch (event.type)
        {
        // Window events
        case SDL_EVENT_QUIT:
            PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_CLOSE);
            break;

        case SDL_EVENT_WINDOW_RESIZED:
        {
            int w, h;
            SDL_GetWindowSizeInPixels(s_sdlWindow, &w, &h);
            nativeWindow.surfaceWidth  = static_cast<uint16>(w);
            nativeWindow.surfaceHeight = static_cast<uint16>(h);

            SDL_GetWindowSize(s_sdlWindow, &w, &h);
            nativeWindow.width  = static_cast<uint16>(w);
            nativeWindow.height = static_cast<uint16>(h);

            // Update Metal layer drawable size
            CAMetalLayer* layer = (__bridge CAMetalLayer*)(nativeWindow.graphicsLayer);
            if (layer)
            {
                layer.drawableSize = CGSizeMake(nativeWindow.surfaceWidth, nativeWindow.surfaceHeight);
            }

            PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_RESIZE);
            break;
        }

        case SDL_EVENT_WINDOW_MINIMIZED:
            nativeWindow.isMinimized = true;
            nativeWindow.isMaximized = false;
            PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_MINIMIZE);
            break;

        case SDL_EVENT_WINDOW_MAXIMIZED:
            nativeWindow.isMaximized = true;
            nativeWindow.isMinimized = false;
            PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_MAXIMIZE);
            break;

        case SDL_EVENT_WINDOW_RESTORED:
            nativeWindow.isMinimized = false;
            nativeWindow.isMaximized = false;
            PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_RESTORE);
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_FOCUS_IN);
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            PushNativeEvent(&nativeWindow, NativeEvent::Type::WINDOW_FOCUS_OUT);
            break;

        // Keyboard events
        case SDL_EVENT_KEY_DOWN:
        {
            uint8 keyCode = MapSDLScancodeToButton(event.key.scancode);
            if (keyCode > 0 && keyCode < static_cast<uint8>(Input::Button::COUNT))
            {
                Input::s_button[keyCode].isPressed   = 1;
                Input::s_button[keyCode].repeatCount = event.key.repeat ? 1 : 0;
            }
            break;
        }

        case SDL_EVENT_KEY_UP:
        {
            uint8 keyCode = MapSDLScancodeToButton(event.key.scancode);
            if (keyCode > 0 && keyCode < static_cast<uint8>(Input::Button::COUNT))
            {
                Input::s_button[keyCode].isPressed   = 0;
                Input::s_button[keyCode].repeatCount = 0;
            }
            break;
        }

        // Mouse button events
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
                Input::s_button[static_cast<uint8>(Input::Button::MOUSE_LEFT)].isPressed = 1;
                break;
            case SDL_BUTTON_RIGHT:
                Input::s_button[static_cast<uint8>(Input::Button::MOUSE_RIGHT)].isPressed = 1;
                break;
            case SDL_BUTTON_MIDDLE:
                Input::s_button[static_cast<uint8>(Input::Button::MOUSE_MIDDLE)].isPressed = 1;
                break;
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            switch (event.button.button)
            {
            case SDL_BUTTON_LEFT:
                Input::s_button[static_cast<uint8>(Input::Button::MOUSE_LEFT)].isPressed = 0;
                break;
            case SDL_BUTTON_RIGHT:
                Input::s_button[static_cast<uint8>(Input::Button::MOUSE_RIGHT)].isPressed = 0;
                break;
            case SDL_BUTTON_MIDDLE:
                Input::s_button[static_cast<uint8>(Input::Button::MOUSE_MIDDLE)].isPressed = 0;
                break;
            }
            break;
        }

        // Mouse motion
        case SDL_EVENT_MOUSE_MOTION:
            Input::s_move.xPoint  = static_cast<uint16>(event.motion.x);
            Input::s_move.yPoint  = static_cast<uint16>(event.motion.y);
            Input::s_move.isMoved = 1;
            break;

        // Mouse wheel
        case SDL_EVENT_MOUSE_WHEEL:
            // SDL wheel values are smaller, scale to match Windows WHEEL_DELTA (120)
            Input::s_scroll.vOffset    = static_cast<int16>(event.wheel.y * 120.0f);
            Input::s_scroll.hOffset    = static_cast<int16>(event.wheel.x * 120.0f);
            Input::s_scroll.isScrolled = 1;
            break;
        }
    }
#else
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
#endif
}

void SetNativeWindowSizeInternal(uint16 width, uint16 height)
{
#ifdef __SDL__
    if (s_sdlWindow)
    {
        SDL_SetWindowSize(s_sdlWindow, width, height);
    }
#else
    // Native implementation (empty)
#endif
}

void GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight)
{
#ifdef __SDL__
    if (s_sdlWindow)
    {
        int w, h;
        SDL_GetWindowSize(s_sdlWindow, &w, &h);
        outWidth  = static_cast<uint16>(w);
        outHeight = static_cast<uint16>(h);
    }
#else
    // Native implementation (empty)
#endif
}

void SetNativePreEventHandler(void* fnHandler)
{
#ifdef __SDL__
    s_preEventHandler = reinterpret_cast<bool (*)(SDL_Event*)>(fnHandler);
#else
    // empty.
#endif
}

HS_NS_END
