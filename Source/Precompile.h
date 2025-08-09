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

    #include <vector>
    #include <stack>
    #include <queue>
    #include <map>
    #include <string>
    #include <utility>
    #include <functional>

#else
    #include <stdio.h>
    #include <string.h>
    #include <stdint.h>

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

#ifdef _WIN32
    #pragma warning(disable : 4819)
#endif

typedef int8_t   int8;
typedef uint8_t  uint8;
typedef int16_t  int16;
typedef uint16_t uint16;
typedef int32_t  int32;
typedef uint32_t uint32;
typedef int64_t  int64;
typedef uint64_t uint64;

#if defined(__APPLE__)
    #if __has_feature(objc_arc)
        #define HS_OBJC_ARC
    #endif
#endif

#ifndef HS_DEBUG_BREAK
    #if defined(__APPLE__)
        #define HS_DEBUG_BREAK() __builtin_trap()
    #else
        #define HS_DEBUG_BREAK() __debugbreak()
    #endif
#endif

#if defined(HS_API_EXPORT)
    #if defined(__APPLE__)
        #define HS_API __attribute__((__visibility__("default")))

        #if defined(HS_HAL_API_EXPORT)
            #define HS_HAL_API __attribute__((__visibility__("default")))
        #endif

        #if defined(HS_CORE_API_EXPORT)
            #define HS_CORE_API __attribute__((__visibility__("default")))
        #endif

        #if defined(HS_RESOURCE_API_EXPORT)
            #define HS_RESOURCE_API __attribute__((__visibility__("default")))
        #endif

        #if defined(HS_SHADERSYSTEM_API_EXPORT)
            #define HS_SHADERSYSTEM_API __attribute__((__visibility__("default")))
        #endif

        #if defined(HS_RHI_API_EXPORT)
            #define HS_RHI_API __attribute__((__visibility__("default")))
        #endif

        #if defined(HS_RENDERER_API_EXPORT)
            #define HS_RENDERER_API __attribute__((__visibility__("default")))
        #endif

        #if defined(HS_ECS_API_EXPORT)
            #define HS_ECS_API __attribute__((__visibility__("default")))
        #endif

        #if defined(HS_EDITOR_API_EXPORT)
            #define HS_ECS_API __attribute__((__visibility__("default")))
        #endif
    #else
        #define HS_API __declspec(dllexport)

        #if defined(HS_HAL_API_EXPORT)
            #define HS_HAL_API _declspec(dllexport)
        #endif

        #if defined(HS_CORE_API_EXPORT)
            #define HS_CORE_API __declspec(dllexport)
        #endif

        #if defined(HS_RESOURCE_API_EXPORT)
            #define HS_RESOURCE_API __declspec(dllexport)
        #endif

        #if defined(HS_SHADERSYSTEM_API_EXPORT)
            #define HS_SHADERSYSTEM_API __declspec(dllexport)
        #endif

        #if defined(HS_RHI_API_EXPORT)
            #define HS_RHI_API __declspec(dllexport)
        #endif

        #if defined(HS_RENDERER_API_EXPORT)
            #define HS_RENDERER_API __declspec(dllexport)
        #endif

        #if defined(HS_ECS_API_EXPORT)
            #define HS_ECS_API __declspec(dllexport)
        #endif

        #if defined(HS_EDITOR_API_EXPORT)
            #define HS_ECS_API __declspec(dllexport)
        #endif
    #endif
#elif defined(HS_API_IMPORT)
    #if defined(__APPLE__)
        #define HS_API
        #define HS_HAL_API
        #define HS_CORE_API
        #define HS_RESOURCE_API
        #define HS_SHADERSYSTEM_API
        #define HS_RHI_API
        #define HS_RENDERER_API
        #define HS_ECS_API
        #define HS_EDITOR_API
    #else
        #define HS_API __declspec(dllimport)
        #define HS_HAL_API __declspec(dllimport)
        #define HS_CORE_API __declspec(dllimport)
        #define HS_RESOURCE_API __declspec(dllimport)
        #define HS_SHADERSYSTEM_API __declspec(dllimport)
        #define HS_RHI_API __declspec(dllimport)
        #define HS_RENDERER_API __declspec(dllimport)
        #define HS_ECS_API __declspec(dllimport)
        #define HS_EDITOR_API __declspec(dllimport)
    #endif
#else
    #define HS_API
    #define HS_HAL_API
    #define HS_CORE_API
    #define HS_RESOURCE_API
    #define HS_SHADERSYSTEM_API
    #define HS_RHI_API
    #define HS_RENDERER_API
    #define HS_ECS_API
    #define HS_EDITOR_API
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

#define HS_INT8_MAX   (0x0f)
#define HS_UINT8_MAX  (0xff)
#define HS_INT16_MAX  (0x0fff)
#define HS_UINT16_MAX (0xffff)
#define HS_INT32_MAX  (0x0fffffff)
#define HS_UINT32_MAX (0xffffffff)
#define HS_INT64_MAX  (0x0fffffffffffffff)
#define HS_UINT64_MAX (0xffffffffffffffff)

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
template <typename Tp>
using Scoped = std::unique_ptr<Tp>;

template <typename Tp, typename... Args>
constexpr Scoped<Tp> hs_make_scoped(Args&&... args)
{
    return std::make_unique<Tp>(std::forward<Args>(args)...);
}
} // namespace HS

#endif


#endif
