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
    #include <type_traits>
    #include <cassert>
#else
    #include <stdio.h>
    #include <string.h>
    #include <stdint.h>
    #include <assert.h>
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

#if defined(__APPLE__)
    #if defined(HS_RESOURCE_API_EXPORT)
        #define HS_RESOURCE_API __attribute__((__visibility__("default")))
    #else
        #define HS_RESOURCE_API
    #endif

    #if defined(HS_SHADERSYSTEM_API_EXPORT)
        #define HS_SHADERSYSTEM_API __attribute__((__visibility__("default")))
    #else
        #define HS_SHADERSYSTEM_API
    #endif

    #if defined(HS_OBJECT_API_EXPORT)
        #define HS_OBJECT_API __attribute__((__visibility__("default")))
    #else    
        #define HS_OBJECT_API 
    #endif

    #if defined(HS_RENDERER_API_EXPORT)
        #define HS_RENDERER_API __attribute__((__visibility__("default")))
    #else    
        #define HS_RENDERER_API
    #endif

    #if defined(HS_ECS_API_EXPORT)
        #define HS_ECS_API __attribute__((__visibility__("default")))
    #else  
        #define HS_ECS_API
    #endif

    #if defined(HS_ENGINE_API_EXPORT)
        #define HS_ENGINE_API __attribute__((__visibility__("default")))
    #else        
        #define HS_ENGINE_API
    #endif

    #if defined(HS_EDITOR_API_EXPORT)
        #define HS_EDITOR_API __attribute__((__visibility__("default")))
    #else
        #define HS_EDITOR_API
    #endif
#else
    #if defined(HS_RESOURCE_API_EXPORT)
        #define HS_RESOURCE_API __declspec(dllexport)
    #elif defined(HS_RESOURCE_API_IMPORT)
        #define HS_RESOURCE_API __declspec(dllimport)
    #else    
        #define HS_RESOURCE_API
    #endif

    #if defined(HS_SHADERSYSTEM_API_EXPORT)
        #define HS_SHADERSYSTEM_API __declspec(dllexport)
    #elif defined (HS_SHADERSYSTEM_API_IMPORT)
        #define HS_SHADERSYSTEM_API __declspec(dllimport)
    #else
        #define HS_SHADERSYSTEM_API
    #endif

    #if defined(HS_OBJECT_API_EXPORT)
        #define HS_OBJECT_API __declspec(dllexport)
    #elif defined(HS_OBJECT_API_IMPORT)
        #define HS_OBJECT_API __declspec(dllimport)
    #else
        #define HS_OBJECT_API
    #endif

    #if defined(HS_RENDERER_API_EXPORT)
        #define HS_RENDERER_API __declspec(dllexport)
    #elif defined(HS_RENDERER_API_IMPORT)
        #define HS_RENDERER_API __declspec(dllimport)
    #else
        #define HS_RENDERER_API
    #endif

    #if defined(HS_ECS_API_EXPORT)
        #define HS_ECS_API __declspec(dllexport)
    #elif defined(HS_ECS_API_IMPORT)
        #define HS_ECS_API __declspec(dllimport)
    #else
        #define HS_ECS_API
    #endif

    #if defined(HS_ENGINE_API_EXPORT)
        #define HS_ENGINE_API __declspec(dllexport)
    #elif defined(HS_ENGINE_API_IMPORT)        
        #define HS_ENGINE_API __declspec(dllimport)
    #else
        #define HS_ENGINE_API    
    #endif

    #if defined(HS_EDITOR_API_EXPORT)
        #define HS_EDITOR_API __declspec(dllexport)
    #elif defined(HS_EDITOR_API_IMPORT)
        #define HS_EDITOR_API __declspec(dllimport)
    #else
        #define HS_EDITOR_API
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

#define HS_INT8_MAX   (0x0f)
#define HS_UINT8_MAX  (0xff)
#define HS_INT16_MAX  (0x0fff)
#define HS_UINT16_MAX (0xffff)
#define HS_INT32_MAX  (0x0fffffff)
#define HS_UINT32_MAX (0xffffffff)
#define HS_INT64_MAX  (0x0fffffffffffffff)
#define HS_UINT64_MAX (0xffffffffffffffff)

#if defined(__APPLE__)
    #define HS_FORCEINLINE      inline __attribute__((always_inline))
    #define HS_FORCENOINLINE    __attribute__((noinline))
#elif defined(_MSC_VER)
    #define HS_FORCEINLINE      __forceinline
    #define HS_FORCENOINLINE    __declspec(noinline)
#else
    #define HS_FORCEINLINE      inline
    #define HS_FORCENOINLINE    
#endif

#define HS_NS_BEGIN namespace HS {

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
