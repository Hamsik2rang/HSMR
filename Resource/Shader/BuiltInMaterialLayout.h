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
#if !defined(HS_SHADER_FILE)
    #include <glm/glm.hpp>

    using float4 = glm::vec4;
#endif
#endif

#if defined(HS_SHADER_FILE)

#if defined(__APPLE__)
    #define ATTR_NUM(x) [[attribute(x)]]
    #define ATTR_PRE_KEY(x)
    #define ATTR_POST_KEY(x) [[x]]

    #define SM_OUT_POSITION [[position]]
    #define SM_OUT_TARGET(n) [[color(##n)]]]]

    #define SM_IN_POSITION(n) 
    #define SM_IN_TEXCOORD(n) 
    #define SM_IN_NORMAL(n) 
    #define SM_IN_COLOR(n) 
#else
    #define ATTR_NUM(x)
    #define ATTR_PRE_KEY(x) [[vk::location(x)]]
    #define ATTR_POST_KEY(x) 

    #define SM_OUT_POSITION : SV_POSITION
    #define SM_OUT_TARGET : SV_TARGET
    
    #define SM_IN_POSITION(n) : POSITION##n
    #define SM_IN_TEXCOORD(n) : TEXCOORD##n
    #define SM_IN_NORMAL(n) : NORMAL##n
    #define SM_IN_COLOR(n) : COLOR##n
#endif

#else
    #define ATTR_NUM(x)
    #define ATTR_PRE_KEY(x)
    #define ATTR_POST_KEY(x) 

    #define SM_OUT_POSITION
    #define SM_OUT_TARGET

    #define SM_IN_POSITION(n)
    #define SM_IN_TEXCOORD(n)
    #define SM_IN_NORMAL(n)
    #define SM_IN_COLOR(n)
#endif

#if defined(HS_BASIC_SHADER) || !defined(HS_SHADER_FILE)

    #define VSINPUT_BASIC     Basic_vertex_input
    #define FSINPUT_BASIC     Basic_fragment_input
    #define VSENTRY_BASIC     Basic_vertex_main
    #define FSENTRY_BASIC     Basic_fragment_main

struct VSINPUT_BASIC
{
    ATTR_PRE_KEY(0) float4 pos   SM_IN_POSITION(0);
    ATTR_PRE_KEY(1) float4 color SM_IN_COLOR(0);
};

struct FSINPUT_BASIC
{
    float4 pos SM_OUT_POSITION;
    ATTR_PRE_KEY(0) float4 color;
};

#endif

#if defined(HS_PBR_BASIC_SHADER) || !defined(HS_SHADER_FILE)
    #define VSINPUT_PBR_BASIC PBRBasic_vertex_input
    #define FSINPUT_PBR_BASIC PBRBasic_fragment_input
    #define VSENTRY_PBR_BASIC PBRBasic_vertex_main
    #define FSENTRY_PBR_BASIC PBRBasic_fragment_main


#endif

//...'

#endif /* __HS_BUILTIN_MATERIAL_LAYOUT_H__ */
