//
//  Precompile.h
//  HSMR
//
//  Created by Yongsik Im on 2/2/25.
//
#ifndef __HS_PRECOMPILE_H__
#define __HS_PRECOMPILE_H__

#ifdef __cplusplus
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>

#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <string>
#include <utility>
#include <functional>
#include <type_traits>
#include <cassert>
#else
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#endif

#ifndef HS_DEBUG_BREAK
#if defined(__APPLE__)
#define HS_DEBUG_BREAK() __builtin_trap()
#else
#define HS_DEBUG_BREAK() __debugbreak()
#endif
#endif

#ifdef _WIN32
#pragma warning(disable : 4819)
#endif

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

#define HS_INT8_MAX (127)
#define HS_INT8_MIN (-128)

#define HS_CHAR_MAX (127)
#define HS_CHAR_MIN (-128)

#define HS_UINT8_MAX (255)
#define HS_UINT8_MIN (0)

#define HS_INT16_MAX (32767)
#define HS_INT16_MIN (-32768)

#define HS_UINT16_MAX (65535)
#define HS_UINT16_MIN (0)

#define HS_INT32_MAX (2147483647)
#define HS_INT32_MIN (-2147483648)

#define HS_UINT32_MAX (4294967295U)
#define HS_UINT32_MIN (0)

#define HS_INT64_MAX (9223372036854775807LL)
#define HS_INT64_MIN (-9223372036854775808LL)

#define HS_UINT64_MAX (18446744073709551615ULL)
#define HS_UINT64_MIN (0)

#define HS_FLT_MAX (3.402823466e+38F)
#define HS_FLT_MIN (1.175494351e-38F)

#define HS_DBL_MAX (1.7976931348623158e+308)
#define HS_DBL_MIN (2.2250738585072014e-308)

#if defined(__APPLE__)
#if __has_feature(objc_arc)
#define HS_OBJC_ARC
#endif
#endif

#if defined(__APPLE__)

#if defined(HS_API_EXPORT)
#define HS_API __attribute__((__visibility__("default")))
#else
#define HS_API
#endif

#if defined(HS_EDITOR_API_EXPORT)
#define HS_EDITOR_API __attribute__((__visibility__("default")))
#else
#define HS_EDITOR_API
#endif

#if defined(HS_PROFILER_API_EXPORT)
#define HS_PROFILER_API __attribute__((__visibility__("default")))
#else
#define HS_PROFILER_API
#endif

#if defined(HS_APPLICATION_API_EXPORT)
#define HS_APPLICATION_API __attribute__((__visibility__("default")))
#else
#define HS_APPLICATION_API
#endif

#else

#if defined(HS_API_EXPORT)
#define HS_API __declspec(dllexport)
#elif defined(HS_API_IMPORT)
#define HS_API __declspec(dllimport)
#else
#define HS_API
#endif

#if defined(HS_EDITOR_API_EXPORT)
#define HS_EDITOR_API __declspec(dllexport)
#elif defined(HS_EDITOR_API_IMPORT)
#define HS_EDITOR_API __declspec(dllimport)
#else
#define HS_EDITOR_API
#endif

#if defined(HS_PROFILER_API_EXPORT)
#define HS_PROFILER_API __declspec(dllexport)
#elif defined(HS_PROFILER_API_IMPORT)
#define HS_PROFILER_API __declspec(dllimport)
#else
#define HS_PROFILER_API
#endif

#if defined(HS_APPLICATION_API_EXPORT)
#define HS_APPLICATION_API __declspec(dllexport)
#elif defined(HS_APPLICATION_API_IMPORT)
#define HS_APPLICATION_API __declspec(dllimport)
#else
#define HS_APPLICATION_API
#endif
#endif

#if defined(__APPLE__)
#define HS_DIR_SEPERATOR '/'
#else
#define HS_DIR_SEPERATOR '\\'
#endif

#define HS_CHAR_INIT_LENGTH       512
#define HS_CHAR_INIT_SHORT_LENGTH 256
#define HS_CHAR_INIT_LONG_LENGTH  1024

#define HS_STRINGIFY(x) #x
#define HS_TO_STRING(x) HS_STRINGIFY(x)

#define HS_BIT(x) ((uint64)1 << (x))

#if defined(__APPLE__)
#define HS_FORCEINLINE   inline __attribute__((always_inline))
#define HS_FORCENOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define HS_FORCEINLINE   __forceinline
#define HS_FORCENOINLINE __declspec(noinline)
#else
#define HS_FORCEINLINE inline
#define HS_FORCENOINLINE
#endif

#define HS_NS_BEGIN \
    namespace hs    \
    {

#define HS_NS_END }

#define HS_NS_EDITOR_BEGIN \
    namespace hs           \
    {                      \
    namespace editor       \
    {

#define HS_NS_EDITOR_END \
    }                    \
    }

#ifdef __cplusplus

#include <memory>

namespace hs
{
template <typename Tp>
using Scoped = std::unique_ptr<Tp>;

template <typename Tp, typename... Args>
constexpr Scoped<Tp> MakeScoped(Args&&... args)
{
    return std::make_unique<Tp>(std::forward<Args>(args)...);
}
} // namespace hs

#endif

#endif
