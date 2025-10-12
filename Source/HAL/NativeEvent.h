//
//  NativeEvent.h
//  HAL
//
//  Created by Yongsik Im on 5/16/2025
//
#ifndef __HS_NATIVE_EVENT_H__
#define __HS_NATIVE_EVENT_H__

#include "Precompile.h"

HS_NS_BEGIN

struct NativeEvent
{
    enum class Type : uint8
    {
        NONE = 0,

        WINDOW_OPEN       = 0x01,
        WINDOW_CLOSE      = 0x02,
        WINDOW_RESIZE     = 0x03,
        WINDOW_MOVE_ENTER = 0x04,
        WINDOW_MOVE_EXIT  = 0x05,
        WINDOW_MOVE       = 0x06,
        WINDOW_MINIMIZE   = 0x07,
        WINDOW_MAXIMIZE   = 0x08,
        WINDOW_FOCUS_IN   = 0x09,
        WINDOW_FOCUS_OUT  = 0x0A,
        WINDOW_RESTORE    = 0x0B,

        BUTTON_PRESS   = 0x10,
        BUTTON_RELEASE = 0x11,

        MOUSE_MOVE   = 0x20,
        MOUSE_SCROLL = 0x21,

        // 0x30 ~ 0xEF : reserve to future use
    };

    NativeEvent() = default;
    NativeEvent(NativeEvent::Type type)
        : type(type)
        , value(0)
    {}

    Type type;
    uint64 value;

    HS_FORCEINLINE bool operator=(NativeEvent::Type type)
    {
        this->type = type;
        value      = 0;
    }

    HS_FORCEINLINE bool operator==(const NativeEvent& rhs)
    {
        return type == rhs.type;
    }

    HS_FORCEINLINE bool operator!=(const NativeEvent& rhs)
    {
        return (*this) != rhs;
    }
};

HS_NS_END

#endif
