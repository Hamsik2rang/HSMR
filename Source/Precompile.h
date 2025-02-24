//
//  Precompile.h
//  MetalSamples
//
//  Created by Yongsik Im on 2/2/25.
//
#ifndef __HS_PRECOMPILE_H__
#define __HS_PRECOMPILE_H__

#include <stdint.h>

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

#ifndef HS_DEBUG_BREAK
#define HS_DEBUG_BREAK __builtin_trap
#endif

#if defined(HS_EDITOR_API_EXPORT)
#define HS_EDITOR_API __attribute__((__visibility__("default")))
#elif defined(HS_EDITOR_API_IMPORT)
#define HS_EDITOR_API
#else
#define HS_EDITOR_API
#endif

#define HS_DIR_SEPERATOR '/'

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

#if defined(__GNUC__) || defined(__clang__)
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
    using Scope = std::unique_ptr<Tp>;

    template <typename Tp, typename ...Args>
    constexpr Scope<Tp> CreateScope(Args&& ... args)
    {
        return std::make_unique<Tp>(std::forward<Args>(args)...);
    }

}

#endif

#endif
