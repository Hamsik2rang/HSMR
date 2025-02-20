//
//  Log.hpp
//  Engine
//
//  Created by Yongsik Im on 2/6/25.
//

#ifndef __HS_LOG_H__
#define __HS_LOG_H__

#include "Precompile.h"

#include <cstdio>
#include <cstdarg>

HS_NS_BEGIN

class Log
{
public:
    enum class EType
    {
        INFO,
        DEBUG,
        WARNING,
        ERROR,
        CRASH,
        ASSERT
    };

    static void Print(const char* file, const uint32 line, const Log::EType type, const char* fmt, ...);

private:
    static void print(const char* file, const uint32 line, const char* start, const char* tag, const char* end, const char* fmt, va_list ptr);
};

namespace LogSymbol
{
    constexpr static Log::EType info = Log::EType::INFO;
    constexpr static Log::EType debug = Log::EType::DEBUG;
    constexpr static Log::EType warning = Log::EType::WARNING;
    constexpr static Log::EType error = Log::EType::ERROR;
    constexpr static Log::EType crash = Log::EType::CRASH;
};

#define HS_LOG(symbol, fmt, ...)                                                       \
    do                                                                                 \
    {                                                                                  \
        HS::Log::Print(__FILE__, __LINE__, HS::LogSymbol::symbol, fmt, ##__VA_ARGS__); \
        if (HS::LogSymbol::symbol == HS::Log::EType::CRASH)                             \
        {                                                                              \
            HS_DEBUG_BREAK();                                                          \
        }                                                                              \
    } while (0)

#ifdef _DEBUG
#define HS_ASSERT(x, fmt, ...) do { const volatile bool b = (x); if (!b) { HS::Log::Print(__FILE__, __LINE__, HS::Log::EType::ASSERT, fmt, ##__VA_ARGS__); HS_DEBUG_BREAK(); } } while(0)
#define HS_CHECK(x, msg) do { const volatile bool b = (x); if (!b) { HS::Log::Print(__FILE__, __LINE__, HS::Log::EType::CRASH, msg); } } while(0)

#else

#define HS_ASSERT(x, fmt, ...)
#define HS_CHECK(x, msg)

#endif

HS_NS_END

#endif /* Log_hpp */
