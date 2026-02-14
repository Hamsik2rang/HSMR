//
//  Input.h
//  Core
//
//  Created by Yongsik Im on 2/10/2025
//
#ifndef __HS_INPUT_H__
#define __HS_INPUT_H__

#include "Precompile.h"

HS_NS_BEGIN

class HS_API Input
{
public:
    enum class Button : uint8
    {
        Unknown = 0x00,

        MouseLeft   = 0x01,
        MouseRight  = 0x02,
        MouseMiddle = 0x04,

        Back    = 0x08,
        Tab     = 0x09,
        Shift   = 0x10,
        Control = 0x11,
        Alt     = 0x12,
        Space   = 0x20,
        End     = 0x23,
        Home    = 0x24,
        Left    = 0x25,
        Up      = 0x26,
        Right   = 0x27,
        Down    = 0x28,
        Insert  = 0x2D,
        Delete  = 0x2E,

        Num0 = 0x30,
        Num1 = 0x31,
        Num2 = 0x32,
        Num3 = 0x33,
        Num4 = 0x34,
        Num5 = 0x35,
        Num6 = 0x36,
        Num7 = 0x37,
        Num8 = 0x38,
        Num9 = 0x39,

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

        LwinOrCommand = 0x5B,
        Rwin          = 0x5C,
        Apps          = 0x5D,
        Sleep         = 0x5F,
        Numpad0       = 0x60,
        Numpad1       = 0x61,
        Numpad2       = 0x62,
        Numpad3       = 0x63,
        Numpad4       = 0x64,
        Numpad5       = 0x65,
        Numpad6       = 0x66,
        Numpad7       = 0x67,
        Numpad8       = 0x68,
        Numpad9       = 0x69,
        Multiply      = 0x6A,
        Add           = 0x6B,
        Separator     = 0x6C,
        Subtract      = 0x6D,
        Decimal       = 0x6E,
        Divide        = 0x6F,
        F1            = 0x70,
        F2            = 0x71,
        F3            = 0x72,
        F4            = 0x73,
        F5            = 0x74,
        F6            = 0x75,
        F7            = 0x76,
        F8            = 0x77,
        F9            = 0x78,
        F10           = 0x79,
        F11           = 0x7A,
        F12           = 0x7B,

        Count
    };

    enum class Mouse : uint8
    {

        Move   = 0x08,
        Scroll = 0x09,

        Count
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
    } s_button[static_cast<uint8>(Button::Count)];

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

#endif
