//
//  Precompile.h
//  HSMR
//
//  Created by Yongsik Im on 2/2/25.
//
#ifndef __HS_PRECOMPILE_H__
#define __HS_PRECOMPILE_H__

#if defined __cplusplus
#include <cstdint>
#include <vector>
#include <string>

#else
#include <stdint.h>
#include <cstring>

#endif

#ifdef _WIN32
#pragma warning(disable: 4819)
#endif


typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

#ifndef HS_DEBUG_BREAK
#if defined(__APPLE__)
#define HS_DEBUG_BREAK() __builtin_trap()
#else
#define HS_DEBUG_BREAK() __debugbreak()
#endif

#endif

#if defined(HS_EDITOR_API_EXPORT)
    #if defined(__APPLE__)
        #define HS_EDITOR_API __attribute__((__visibility__("default")))
    #else
        #define HS_EDITOR_API __declspec(dllexport)
    #endif
#elif defined(HS_EDITOR_API_IMPORT)
    #if defined(__APPLE__)
        #define HS_EDITOR_API
    #else
        #define HS_EDITOR_API __declspec(dllimport)
    #endif
#else
    #define HS_EDITOR_API
#endif

#if defined(__APPLE__)
    #define HS_DIR_SEPERATOR '/'
#else
    #define HS_DIR_SEPERATOR '\\'
#endif

#define HS_CHAR_INIT_LENGTH 512
#define HS_CHAR_INIT_SHORT_LENGTH 256
#define HS_CHAR_INIT_LONG_LENGTH 1024

#define HS_STRINGIFY(x) #x
#define HS_TO_STRING(x) HS_STRINGIFY(x)


#define HS_BIT(x) ((uint64)1 << (x))

#define HS_INT8_MAX     (0x0f)
#define HS_UINT8_MAX    (0xff)
#define HS_INT16_MAX    (0x0fff)
#define HS_UINT16_MAX   (0xffff)
#define HS_INT32_MAX    (0x0fffffff)
#define HS_UINT32_MAX   (0xffffffff)
#define HS_INT64_MAX    (0x0fffffffffffffff)
#define HS_UINT64_MAX   (0xffffffffffffffff)

#if defined(__APPLE__)
    #define HS_FORCEINLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
    #define HS_FORCEINLINE __forceinline
#else
    #define HS_FORCEINLINE inline
#endif



#define HS_NS_BEGIN \
    namespace HS    \
    {

#define HS_NS_END }

#define HS_NS_EDITOR_BEGIN \
    namespace HS           \
    {                      \
    namespace Editor       \
    {

#define HS_NS_EDITOR_END \
    }                    \
    }

#ifdef __cplusplus

#include <memory>

namespace HS
{
    template<typename Tp>
    using Scoped = std::unique_ptr<Tp>;

    template <typename Tp, typename ...Args>
    constexpr Scoped<Tp> hs_make_scoped(Args&& ... args)
    {
        return std::make_unique<Tp>(std::forward<Args>(args)...);
    }
}

#endif

#endif
