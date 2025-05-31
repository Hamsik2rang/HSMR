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

    #define VSINPUT_BASIC     Basic_vertex_input
    #define FSINPUT_BASIC     Basic_fragment_input
    #define VSENTRY_BASIC     Basic_vertex_main
    #define FSENTRY_BASIC     Basic_fragment_main

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

#if defined(HS_PBR_BASIC_SHADER) || !defined(HS_SHADER_FILE)
    #define VSINPUT_PBR_BASIC PBRBasic_vertex_input
    #define FSINPUT_PBR_BASIC PBRBasic_fragment_input
    #define VSENTRY_PBR_BASIC PBRBasic_vertex_main
    #define FSENTRY_PBR_BASIC PBRBasic_fragment_main
//
//
//struct VSINPUT_PBR_BASIC
//{
//    float4 albedo;
//    float metallic;
//    float roughness;
//    float ao;
//    float4 emmisive;
//};
//
//struct FSINPUT_PBR_BASIC
//{
//    
//};

#endif

//...'

#endif /* __HS_BUILTIN_MATERIAL_LAYOUT_H__ */
