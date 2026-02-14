//
//  Log.hpp
//  Core
//
//  Created by Yongsik Im on 2/6/2025
//

#ifndef __HS_LOG_H__
#define __HS_LOG_H__

#include "Precompile.h"
#include "Core/Exception.h"

#include <cstdio>
#include <cstdarg>
#include <stdexcept>

HS_NS_BEGIN

class HS_API Log
{
public:
    enum class EType
    {
        Info,
        Debug,
        Warning,
        Error,
        Crash,
        Assert
    };

    static void Print(const char* file, const uint32 line, const Log::EType type, const char* fmt, ...);

private:
    static void print(const char* file, const uint32 line, const char* start, const char* tag, const char* end, const char* fmt, va_list ptr);
};

namespace LogSymbol
{
    constexpr static Log::EType info = Log::EType::Info;
    constexpr static Log::EType debug = Log::EType::Debug;
    constexpr static Log::EType warning = Log::EType::Warning;
    constexpr static Log::EType error = Log::EType::Error;
    constexpr static Log::EType crash = Log::EType::Crash;
};

#define HS_LOG(symbol, fmt, ...)                                                       \
    do                                                                                 \
    {                                                                                  \
        hs::Log::Print(__FILE__, __LINE__, hs::LogSymbol::symbol, fmt, ##__VA_ARGS__); \
        if (hs::LogSymbol::symbol == hs::Log::EType::Crash)                             \
        {                                                                              \
            HS_DEBUG_BREAK();                                                          \
        }                                                                              \
    } while (0)

#ifdef _DEBUG

#define HS_ASSERT(x, fmt, ...) do { const volatile bool b = !!(x); if (!b) { hs::Log::Print(__FILE__, __LINE__, hs::Log::EType::Assert, fmt, ##__VA_ARGS__); HS_DEBUG_BREAK(); } } while(0)
#define HS_CHECK(x, msg) do { const volatile bool b = !!(x); if (!b) { hs::Log::Print(__FILE__, __LINE__, hs::Log::EType::Crash, msg); } } while(0)

#define HS_THROW(fmt, ...) \
    do { \
        hs::Log::Print(__FILE__, __LINE__, hs::LogSymbol::crash, fmt, ##__VA_ARGS__); \
        HS_DEBUG_BREAK(); \
        throw hs::Exception(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (false)


#else

#define HS_ASSERT(x, fmt, ...)
#define HS_CHECK(x, msg)

#define HS_THROW(fmt, ...) \
    do { \
        hs::Log::Print( __FILE__, __LINE__, hs::LogSymbol::crash, fmt, ##__VA_ARGS__); \
        throw hs::Exception(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (false)

#endif

HS_NS_END

#endif /* Log_hpp */
