#include "Engine/Core/Log.h"

HS_NS_BEGIN
#if defined(__APPLE__)
static const char* log_color_red = "🔴";
static const char* log_color_green = "🟢";
static const char* log_color_yellow = "🟡";
static const char* log_color_cyan = "🔵";
static const char* log_color_white = "⚪️";
static const char* log_color_magenta = "🟣";
static const char* log_color_reset = "";
#else
static const char* log_color_red = "\x1b[31m";
static const char* log_color_green = "\x1b[32m";
static const char* log_color_yellow = "\x1b[33m";
static const char* log_color_cyan = "\x1b[34m";
static const char* log_color_white = "\x1b[37m";
static const char* log_color_magenta = "\x1b[35m";
static const char* log_color_reset = "\x1b[0m";

#endif
static const char* log_tag_info = "[INFO]";
static const char* log_tag_debug = "[DEBUG]";
static const char* log_tag_warning = "[WARNING]";
static const char* log_tag_error = "[ERROR]";
static const char* log_tag_crash = "[CRASH]";
static const char* log_tag_assert = "[ASSERT]";
static const char* log_tag_invalid = "[??????]";

void Log::Print(const char* file, const uint32 line, const Log::EType type, const char* fmt, ...)
{
    const char* start = nullptr;
    const char* end = log_color_reset;
    const char* tag = nullptr;

    switch (type)
    {
        case EType::LOG_INFO:
        {
            start = log_color_green;
            tag = log_tag_info;
        }
        break;
        case EType::LOG_DEBUG:
        {
            start = log_color_white;
            tag = log_tag_debug;
            break;
        }
        case EType::LOG_WARNING:
        {
            start = log_color_yellow;
            tag = log_tag_warning;
            break;
        }
        case EType::LOG_ERROR:
        {
            start = log_color_red;
            tag = log_tag_error;
            break;
        }
        case EType::LOG_CRASH:
        {
            start = log_color_red;
            tag = log_tag_crash;
            break;
        }
        case EType::LOG_ASSERT:
        {
            start = log_color_magenta;
            tag = log_tag_assert;
            break;
        }
        default:
        {
            start = log_color_white;
            tag = log_tag_invalid;
            break;
        }
    }

    va_list ptr;
    va_start(ptr, fmt);
    print(file, line, start, tag, end, fmt, ptr);
    va_end(ptr);
}

void Log::print(const char* file, const uint32 line, const char* start, const char* tag, const char* end, const char* fmt, va_list ptr)
{
    fprintf(stdout, " %s%s ",start, tag);
    vfprintf(stdout, fmt, ptr);
    fprintf(stdout, " (%s:%u)%s\n", file, line, end);
}

HS_NS_END
