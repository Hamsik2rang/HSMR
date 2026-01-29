//
//  SDLPlatform.cpp
//  Platform
//
//  Created for HSMR Lightweight Prototyping Framework
//

#include "Platform/SDL/SDLPlatform.h"

#ifdef __SDL__

#include "Core/HAL/Input.h"
#include "Core/Log.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

HS_NS_BEGIN

static SDL_Window* s_sdlWindow = nullptr;
static NativeWindow* s_boundHsWindow = nullptr;
static bool s_sdlInitialized = false;

// Pre-event handler for ImGui integration
static bool (*s_preEventHandler)(SDL_Event*) = nullptr;

// Get SDL window for Vulkan surface creation
SDL_Window* GetSDLWindow()
{
    return s_sdlWindow;
}

// Map SDL scancode to Input::Button enum value
static uint8 MapSDLScancodeToButton(SDL_Scancode scancode)
{
    using Button = Input::Button;

    switch (scancode)
    {
        // Letters A-Z
        case SDL_SCANCODE_A: return static_cast<uint8>(Button::A);
        case SDL_SCANCODE_B: return static_cast<uint8>(Button::B);
        case SDL_SCANCODE_C: return static_cast<uint8>(Button::C);
        case SDL_SCANCODE_D: return static_cast<uint8>(Button::D);
        case SDL_SCANCODE_E: return static_cast<uint8>(Button::E);
        case SDL_SCANCODE_F: return static_cast<uint8>(Button::F);
        case SDL_SCANCODE_G: return static_cast<uint8>(Button::G);
        case SDL_SCANCODE_H: return static_cast<uint8>(Button::H);
        case SDL_SCANCODE_I: return static_cast<uint8>(Button::I);
        case SDL_SCANCODE_J: return static_cast<uint8>(Button::J);
        case SDL_SCANCODE_K: return static_cast<uint8>(Button::K);
        case SDL_SCANCODE_L: return static_cast<uint8>(Button::L);
        case SDL_SCANCODE_M: return static_cast<uint8>(Button::M);
        case SDL_SCANCODE_N: return static_cast<uint8>(Button::N);
        case SDL_SCANCODE_O: return static_cast<uint8>(Button::O);
        case SDL_SCANCODE_P: return static_cast<uint8>(Button::P);
        case SDL_SCANCODE_Q: return static_cast<uint8>(Button::Q);
        case SDL_SCANCODE_R: return static_cast<uint8>(Button::R);
        case SDL_SCANCODE_S: return static_cast<uint8>(Button::S);
        case SDL_SCANCODE_T: return static_cast<uint8>(Button::T);
        case SDL_SCANCODE_U: return static_cast<uint8>(Button::U);
        case SDL_SCANCODE_V: return static_cast<uint8>(Button::V);
        case SDL_SCANCODE_W: return static_cast<uint8>(Button::W);
        case SDL_SCANCODE_X: return static_cast<uint8>(Button::X);
        case SDL_SCANCODE_Y: return static_cast<uint8>(Button::Y);
        case SDL_SCANCODE_Z: return static_cast<uint8>(Button::Z);

        // Numbers 0-9
        case SDL_SCANCODE_0: return static_cast<uint8>(Button::NUM_0);
        case SDL_SCANCODE_1: return static_cast<uint8>(Button::NUM_1);
        case SDL_SCANCODE_2: return static_cast<uint8>(Button::NUM_2);
        case SDL_SCANCODE_3: return static_cast<uint8>(Button::NUM_3);
        case SDL_SCANCODE_4: return static_cast<uint8>(Button::NUM_4);
        case SDL_SCANCODE_5: return static_cast<uint8>(Button::NUM_5);
        case SDL_SCANCODE_6: return static_cast<uint8>(Button::NUM_6);
        case SDL_SCANCODE_7: return static_cast<uint8>(Button::NUM_7);
        case SDL_SCANCODE_8: return static_cast<uint8>(Button::NUM_8);
        case SDL_SCANCODE_9: return static_cast<uint8>(Button::NUM_9);

        // Function keys
        case SDL_SCANCODE_F1:  return static_cast<uint8>(Button::F1);
        case SDL_SCANCODE_F2:  return static_cast<uint8>(Button::F2);
        case SDL_SCANCODE_F3:  return static_cast<uint8>(Button::F3);
        case SDL_SCANCODE_F4:  return static_cast<uint8>(Button::F4);
        case SDL_SCANCODE_F5:  return static_cast<uint8>(Button::F5);
        case SDL_SCANCODE_F6:  return static_cast<uint8>(Button::F6);
        case SDL_SCANCODE_F7:  return static_cast<uint8>(Button::F7);
        case SDL_SCANCODE_F8:  return static_cast<uint8>(Button::F8);
        case SDL_SCANCODE_F9:  return static_cast<uint8>(Button::F9);
        case SDL_SCANCODE_F10: return static_cast<uint8>(Button::F10);
        case SDL_SCANCODE_F11: return static_cast<uint8>(Button::F11);
        case SDL_SCANCODE_F12: return static_cast<uint8>(Button::F12);

        // Special keys
        case SDL_SCANCODE_BACKSPACE: return static_cast<uint8>(Button::BACK);
        case SDL_SCANCODE_TAB:       return static_cast<uint8>(Button::TAB);
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:    return static_cast<uint8>(Button::SHIFT);
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:     return static_cast<uint8>(Button::CONTROL);
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_RALT:      return static_cast<uint8>(Button::ALT);
        case SDL_SCANCODE_SPACE:     return static_cast<uint8>(Button::SPACE);
        case SDL_SCANCODE_END:       return static_cast<uint8>(Button::END);
        case SDL_SCANCODE_HOME:      return static_cast<uint8>(Button::HOME);
        case SDL_SCANCODE_LEFT:      return static_cast<uint8>(Button::LEFT);
        case SDL_SCANCODE_UP:        return static_cast<uint8>(Button::UP);
        case SDL_SCANCODE_RIGHT:     return static_cast<uint8>(Button::RIGHT);
        case SDL_SCANCODE_DOWN:      return static_cast<uint8>(Button::DOWN);
        case SDL_SCANCODE_INSERT:    return static_cast<uint8>(Button::INSERT);
        case SDL_SCANCODE_DELETE:    return static_cast<uint8>(Button::DELETE);

        // Numpad
        case SDL_SCANCODE_KP_0: return static_cast<uint8>(Button::NUMPAD_0);
        case SDL_SCANCODE_KP_1: return static_cast<uint8>(Button::NUMPAD_1);
        case SDL_SCANCODE_KP_2: return static_cast<uint8>(Button::NUMPAD_2);
        case SDL_SCANCODE_KP_3: return static_cast<uint8>(Button::NUMPAD_3);
        case SDL_SCANCODE_KP_4: return static_cast<uint8>(Button::NUMPAD_4);
        case SDL_SCANCODE_KP_5: return static_cast<uint8>(Button::NUMPAD_5);
        case SDL_SCANCODE_KP_6: return static_cast<uint8>(Button::NUMPAD_6);
        case SDL_SCANCODE_KP_7: return static_cast<uint8>(Button::NUMPAD_7);
        case SDL_SCANCODE_KP_8: return static_cast<uint8>(Button::NUMPAD_8);
        case SDL_SCANCODE_KP_9: return static_cast<uint8>(Button::NUMPAD_9);
        case SDL_SCANCODE_KP_MULTIPLY: return static_cast<uint8>(Button::MULTIPLY);
        case SDL_SCANCODE_KP_PLUS:     return static_cast<uint8>(Button::ADD);
        case SDL_SCANCODE_KP_MINUS:    return static_cast<uint8>(Button::SUBTRACT);
        case SDL_SCANCODE_KP_PERIOD:   return static_cast<uint8>(Button::DECIMAL);
        case SDL_SCANCODE_KP_DIVIDE:   return static_cast<uint8>(Button::DIVIDE);

        // Windows/Command key
        case SDL_SCANCODE_LGUI: return static_cast<uint8>(Button::LWIN_OR_COMMAND);
        case SDL_SCANCODE_RGUI: return static_cast<uint8>(Button::RWIN);

        default: return static_cast<uint8>(Button::UNKNOWN);
    }
}

bool CreateNativeWindowInternal(const char* name, uint16 width, uint16 height, EWindowFlags flag, NativeWindow& outNativeWindow)
{
    // Initialize SDL if not already done
    if (!s_sdlInitialized)
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
        {
            HS_LOG(crash, "Failed to initialize SDL: %s", SDL_GetError());
            return false;
        }
        s_sdlInitialized = true;
    }

    // Convert EWindowFlags to SDL_WindowFlags
    // EWindowFlags is already designed to match SDL_WindowFlags
    SDL_WindowFlags sdlFlags = static_cast<SDL_WindowFlags>(static_cast<uint64>(flag));

    // Ensure Vulkan flag is set for surface creation
    sdlFlags |= SDL_WINDOW_VULKAN;

    // Create SDL window
    s_sdlWindow = SDL_CreateWindow(name, width, height, sdlFlags);
    if (!s_sdlWindow)
    {
        HS_LOG(crash, "Failed to create SDL window: %s", SDL_GetError());
        return false;
    }

    s_boundHsWindow = &outNativeWindow;

    // Get actual drawable size (may differ on high-DPI displays)
    int drawableWidth, drawableHeight;
    SDL_GetWindowSizeInPixels(s_sdlWindow, &drawableWidth, &drawableHeight);

    float displayScale = SDL_GetWindowDisplayScale(s_sdlWindow);

    // Populate NativeWindow struct
    outNativeWindow = {};
    outNativeWindow.width = width;
    outNativeWindow.height = height;
    outNativeWindow.surfaceWidth = static_cast<uint16>(drawableWidth);
    outNativeWindow.surfaceHeight = static_cast<uint16>(drawableHeight);
    outNativeWindow.flags = flag | EWindowFlags::WINDOW_VULKAN;
    outNativeWindow.title = name;
    outNativeWindow.scale = displayScale;
    outNativeWindow.handle = s_sdlWindow;
    outNativeWindow.isMaximized = (static_cast<uint64>(flag) & SDL_WINDOW_MAXIMIZED) != 0;
    outNativeWindow.isMinimized = (static_cast<uint64>(flag) & SDL_WINDOW_MINIMIZED) != 0;
    outNativeWindow.resizable = (flag & EWindowFlags::WINDOW_RESIZABLE) != EWindowFlags::NONE;
    outNativeWindow.useHDR = (flag & EWindowFlags::WINDOW_HIGH_PIXEL_DENSITY) != EWindowFlags::NONE;

    HS_LOG(info, "[SDL] Window created: %s (%dx%d, surface: %dx%d, scale: %.2f)",
           name, width, height, drawableWidth, drawableHeight, displayScale);

    return true;
}

void DestroyNativeWindowInternal(NativeWindow& nativeWindow)
{
    if (s_sdlWindow)
    {
        SDL_DestroyWindow(s_sdlWindow);
        s_sdlWindow = nullptr;
    }

    if (s_sdlInitialized)
    {
        SDL_Quit();
        s_sdlInitialized = false;
    }

    s_boundHsWindow = nullptr;
}

void ShowNativeWindowInternal(const NativeWindow& nativeWindow)
{
    if (s_sdlWindow)
    {
        SDL_ShowWindow(s_sdlWindow);
    }
}

void PollNativeEventInternal(NativeWindow& nativeWindow)
{
    // Reset per-frame input states
    Input::s_move.isMoved = 0;
    Input::s_scroll.isScrolled = 0;

    s_boundHsWindow = &nativeWindow;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // Let ImGui process the event first
        if (s_preEventHandler)
        {
            s_preEventHandler(&event);
        }

        // Always process window events, only skip input events if ImGui captured them
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
                nativeWindow.surfaceWidth = static_cast<uint16>(w);
                nativeWindow.surfaceHeight = static_cast<uint16>(h);

                SDL_GetWindowSize(s_sdlWindow, &w, &h);
                nativeWindow.width = static_cast<uint16>(w);
                nativeWindow.height = static_cast<uint16>(h);

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
                    Input::s_button[keyCode].isPressed = 1;
                    Input::s_button[keyCode].repeatCount = event.key.repeat ? 1 : 0;
                }
                break;
            }

            case SDL_EVENT_KEY_UP:
            {
                uint8 keyCode = MapSDLScancodeToButton(event.key.scancode);
                if (keyCode > 0 && keyCode < static_cast<uint8>(Input::Button::COUNT))
                {
                    Input::s_button[keyCode].isPressed = 0;
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
                Input::s_move.xPoint = static_cast<uint16>(event.motion.x);
                Input::s_move.yPoint = static_cast<uint16>(event.motion.y);
                Input::s_move.isMoved = 1;
                break;

            // Mouse wheel
            case SDL_EVENT_MOUSE_WHEEL:
                // SDL wheel values are smaller, scale to match Windows WHEEL_DELTA (120)
                Input::s_scroll.vOffset = static_cast<int16>(event.wheel.y * 120.0f);
                Input::s_scroll.hOffset = static_cast<int16>(event.wheel.x * 120.0f);
                Input::s_scroll.isScrolled = 1;
                break;
        }
    }
}

void SetNativeWindowSizeInternal(uint16 width, uint16 height)
{
    if (s_sdlWindow)
    {
        SDL_SetWindowSize(s_sdlWindow, width, height);
    }
}

void GetNativeWindowSizeInternal(uint16& outWidth, uint16& outHeight)
{
    if (s_sdlWindow)
    {
        int w, h;
        SDL_GetWindowSize(s_sdlWindow, &w, &h);
        outWidth = static_cast<uint16>(w);
        outHeight = static_cast<uint16>(h);
    }
}

// Platform-dependent function for ImGui integration
void SetNativePreEventHandler(void* fnHandler)
{
    s_preEventHandler = reinterpret_cast<bool(*)(SDL_Event*)>(fnHandler);
}

HS_NS_END

#endif // __SDL__
