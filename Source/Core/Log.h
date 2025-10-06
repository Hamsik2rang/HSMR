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

class Log
{
public:
    enum class EType
    {
        LOG_INFO,
        LOG_DEBUG,
        LOG_WARNING,
        LOG_ERROR,
        LOG_CRASH,
        LOG_ASSERT
    };

    static void Print(const char* file, const uint32 line, const Log::EType type, const char* fmt, ...);

private:
    static void print(const char* file, const uint32 line, const char* start, const char* tag, const char* end, const char* fmt, va_list ptr);
};

namespace LogSymbol
{
    constexpr static Log::EType info = Log::EType::LOG_INFO;
    constexpr static Log::EType debug = Log::EType::LOG_DEBUG;
    constexpr static Log::EType warning = Log::EType::LOG_WARNING;
    constexpr static Log::EType error = Log::EType::LOG_ERROR;
    constexpr static Log::EType crash = Log::EType::LOG_CRASH;
};

#define HS_LOG(symbol, fmt, ...)                                                       \
    do                                                                                 \
    {                                                                                  \
        HS::Log::Print(__FILE__, __LINE__, HS::LogSymbol::symbol, fmt, ##__VA_ARGS__); \
        if (HS::LogSymbol::symbol == HS::Log::EType::LOG_CRASH)                             \
        {                                                                              \
            HS_DEBUG_BREAK();                                                          \
        }                                                                              \
    } while (0)

#ifdef _DEBUG

#define HS_ASSERT(x, fmt, ...) do { const volatile bool b = !!(x); if (!b) { HS::Log::Print(__FILE__, __LINE__, HS::Log::EType::LOG_ASSERT, fmt, ##__VA_ARGS__); HS_DEBUG_BREAK(); } } while(0)
#define HS_CHECK(x, msg) do { const volatile bool b = !!(x); if (!b) { HS::Log::Print(__FILE__, __LINE__, HS::Log::EType::LOG_CRASH, msg); } } while(0)

#define HS_THROW(fmt, ...) \
    do { \
        HS::Log::Print(__FILE__, __LINE__, HS::LogSymbol::crash, fmt, ##__VA_ARGS__); \
        HS_DEBUG_BREAK(); \
        throw HS::Exception(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (false)


#else

#define HS_ASSERT(x, fmt, ...)
#define HS_CHECK(x, msg)

#define HS_THROW(fmt, ...) \
    do { \
        HS::Log::Print( __FILE__, __LINE__, HS::LogSymbol::crash, fmt, ##__VA_ARGS__); \
        throw HS::Exception(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
    } while (false)

#endif

HS_NS_END

#endif /* Log_hpp */
