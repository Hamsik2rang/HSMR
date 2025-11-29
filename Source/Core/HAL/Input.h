//
//  Input.h
//  Core
//
//  Created by Yongsik Im on 2/10/2025
//
#ifndef __HS_INPUT_H__
#define __HS_INPUT_H__

#include "Precompile.h"

#ifdef _WIN32
#pragma push_macro("DELETE")
#undef DELETE
#endif

HS_NS_BEGIN

class HS_API Input
{
public:
    enum class Button : uint8
    {
        UNKNOWN = 0x00,

        MOUSE_LEFT   = 0x01,
        MOUSE_RIGHT  = 0x02,
        MOUSE_MIDDLE = 0x04,

        BACK    = 0x08,
        TAB     = 0x09,
        SHIFT   = 0x10,
        CONTROL = 0x11,
        ALT     = 0x12,
        SPACE   = 0x20,
        END     = 0x23,
        HOME    = 0x24,
        LEFT    = 0x25,
        UP      = 0x26,
        RIGHT   = 0x27,
        DOWN    = 0x28,
        INSERT  = 0x2D,
        DELETE  = 0x2E,

        NUM_0 = 0x30,
        NUM_1 = 0x31,
        NUM_2 = 0x32,
        NUM_3 = 0x33,
        NUM_4 = 0x34,
        NUM_5 = 0x35,
        NUM_6 = 0x36,
        NUM_7 = 0x37,
        NUM_8 = 0x38,
        NUM_9 = 0x39,

        A = 0x41,
        B = 0x42,
        C = 0x43,
        D = 0x44,
        E = 0x45,
        F = 0x46,
        G = 0x47,
        H = 0x48,
        I = 0x49,
        J = 0x4A,
        K = 0x4B,
        L = 0x4C,
        M = 0x4D,
        N = 0x4E,
        O = 0x4F,
        P = 0x50,
        Q = 0x51,
        R = 0x52,
        S = 0x53,
        T = 0x54,
        U = 0x55,
        V = 0x56,
        W = 0x57,
        X = 0x58,
        Y = 0x59,
        Z = 0x5A,

        LWIN_OR_COMMAND = 0x5B,
        RWIN            = 0x5C,
        APPS            = 0x5D,
        SLEEP           = 0x5F,
        NUMPAD_0        = 0x60,
        NUMPAD_1        = 0x61,
        NUMPAD_2        = 0x62,
        NUMPAD_3        = 0x63,
        NUMPAD_4        = 0x64,
        NUMPAD_5        = 0x65,
        NUMPAD_6        = 0x66,
        NUMPAD_7        = 0x67,
        NUMPAD_8        = 0x68,
        NUMPAD_9        = 0x69,
        MULTIPLY        = 0x6A,
        ADD             = 0x6B,
        SEPARATOR       = 0x6C,
        SUBTRACT        = 0x6D,
        DECIMAL         = 0x6E,
        DIVIDE          = 0x6F,
        F1              = 0x70,
        F2              = 0x71,
        F3              = 0x72,
        F4              = 0x73,
        F5              = 0x74,
        F6              = 0x75,
        F7              = 0x76,
        F8              = 0x77,
        F9              = 0x78,
        F10             = 0x79,
        F11             = 0x7A,
        F12             = 0x7B,

        COUNT
    };

    enum class Mouse : uint8
    {

        MOVE   = 0x08,
        SCROLL = 0x09,

        COUNT
    };

    HS_FORCEINLINE static bool IsPressed(Button button) { return static_cast<bool>(s_button[static_cast<uint8>(button)].isPressed); }
    HS_FORCEINLINE static bool IsReleased(Button button) { return !IsPressed(button); }
    HS_FORCEINLINE static bool IsMouseMoved() { return s_move.isMoved; }
    HS_FORCEINLINE static bool IsMouseScrolled() { return s_scroll.isScrolled; }

    HS_FORCEINLINE static void GetMousePosition(uint16& x, uint16& y)
    {
        x = s_move.xPoint;
        y = s_move.yPoint;
    }
    HS_FORCEINLINE static void GetMouseScrollOffset(uint16& vOffset, uint16& hOffset)
    {
        vOffset = s_scroll.vOffset;
        hOffset = s_scroll.hOffset;
    }

    static struct ButtonInfo
    {
        uint32 isPressed   : 1;
        uint32 repeatCount : 15;
    } s_button[static_cast<uint8>(Button::COUNT)];

    static struct MoveInfo
    {
        uint16 xPoint  : 15;
        uint16 yPoint  : 15;
        uint16 isMoved : 1;
    } s_move;

    static struct ScrollInfo
    {
        uint16 vOffset    : 15;
        uint16 hOffset    : 15;
        uint16 isScrolled : 1;
    } s_scroll;
};

HS_NS_END


#ifdef _WIN32
#pragma pop_macro("DELETE")
#endif

#endif