#define HS_SHADER_FILE
#define HS_BASIC_SHADER
#include "BuiltInMaterialLayout.h"

#include <metal_stdlib>

using namespace metal;


vertex FSINPUT_BASIC VertexShader_Basic(
                                        VSINPUT_BASIC input ATTR_KEY(stage_in))
{
    FSInput_Basic output;
    
    output.pos = input.pos;
    output.color = input.color;
    
    return output;
}

fragment float4 FragmentShader_Basic(FSINPUT_BASIC input ATTR_KEY(stage_in))
{
    return input.color;
}
