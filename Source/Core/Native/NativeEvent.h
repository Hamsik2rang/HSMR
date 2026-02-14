//
//  NativeEvent.h
//  Platform
//
//  Created by Yongsik Im on 5/16/2025
//
#ifndef __HS_NATIVE_EVENT_H__
#define __HS_NATIVE_EVENT_H__

#include "Precompile.h"

HS_NS_BEGIN

struct HS_CORE_API NativeEvent
{
    enum class Type : uint8
    {
        None = 0,

        WindowOpen      = 0x01,
        WindowClose     = 0x02,
        WindowResize    = 0x03,
        WindowMoveEnter = 0x04,
        WindowMoveExit  = 0x05,
        WindowMove      = 0x06,
        WindowMinimize  = 0x07,
        WindowMaximize  = 0x08,
        WindowFocusIn   = 0x09,
        WindowFocusOut  = 0x0A,
        WindowRestore   = 0x0B,

        ButtonPress   = 0x10,
        ButtonRelease = 0x11,

        MouseMove   = 0x20,
        MouseScroll = 0x21,

        // 0x30 ~ 0xEF : reserve to future use
    };

    NativeEvent() = default;
    NativeEvent(NativeEvent::Type type)
        : type(type)
        , value(0)
    {}

    Type type;
    uint64 value;

    HS_FORCEINLINE NativeEvent& operator=(const NativeEvent& event)
    {
        this->type = event.type;
        value      = event.value;

        return *this;
    }

    HS_FORCEINLINE NativeEvent& operator=(const NativeEvent::Type& eventType)
    {
        this->type = eventType;
        value      = 0;

        return *this;
    }

    HS_FORCEINLINE bool operator==(const NativeEvent& rhs)
    {
        return type == rhs.type;
    }

    HS_FORCEINLINE bool operator!=(const NativeEvent& rhs)
    {
        return (this->type) != rhs.type;
    }
};

HS_NS_END

#endif
