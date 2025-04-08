//
//  BuiltInMaterialLayout.h
//  Engine
//
//  Created by Yongsik Im on 2/9/25.
//
#ifndef __HS_BUILTIN_MATERIAL_LAYOUT_H__
#define __HS_BUILTIN_MATERIAL_LAYOUT_H__

#if defined(__APPLE__)
#include <simd/simd.h>

using float4 = vector_float4;

#else
#include <glm/glm.hpp>

using float4 = glm::vec4;
#endif

#if defined(HS_SHADER_FILE)

#if defined(__APPLE__)
    #define ATTR_NUM(x) [[attribute(x)]]
    #define ATTR_KEY(x) [[x]]
#else
    #define ATTR_NUM(x) 
    #define ATTR_KEY(x)
#endif

#else
    #define ATTR_NUM(x)
    #define ATTR_KEY(x)
#endif

#if defined(HS_BASIC_SHADER) || !defined(HS_SHADER_FILE)

    #define VSINPUT_BASIC     VSInput_Basic
    #define FSINPUT_BASIC     FSInput_Basic
    #define VSENTRY_BASIC     VertexShader_Basic
    #define FSENTRY_BASIC     FragmentShader_Basic

struct VSINPUT_BASIC
{
    float4 pos   ATTR_NUM(0);
    float4 color ATTR_NUM(1);
};

struct FSINPUT_BASIC
{
    float4 pos ATTR_KEY(position);
    float4 color;
};

#endif

//...'

#endif /* __HS_BUILTIN_MATERIAL_LAYOUT_H__ */
