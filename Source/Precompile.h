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

#define HS_EDITOR_API __attribute__((__visibility__("default"))

#elif defined(HS_EDITOR_API_IMPORT)

#define HS_EDITOR_API

#else

#define HS_EDITOR_API
#endif

#ifndef HS_NS_BEGIN
#define HS_NS_BEGIN \
    namespace HS    \
    {
#endif
#ifndef HS_NS_END
#define HS_NS_END }
#endif
#ifndef HS_NS_EDITOR_BEGIN
#define HS_NS_EDITOR_BEGIN \
    namespace HS           \
    {                      \
    namespace Editor                 \
    {
#endif
#ifndef HS_NS_EDITOR_END
#define HS_NS_EDITOR_END \
    }                    \
    }
#endif

#endif
