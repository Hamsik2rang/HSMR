#define HS_SHADER_FILE
#define HS_BASIC_SHADER
#include "../BuiltInMaterialLayout.h"

#include <metal_stdlib>

using namespace metal;


vertex FSINPUT_BASIC VSENTRY_BASIC(
                                        VSINPUT_BASIC input ATTR_KEY(stage_in))
{
    FSINPUT_BASIC output;
    
    output.pos = input.pos;
    output.color = input.color;
    
    return output;
}

fragment float4 FSENTRY_BASIC(FSINPUT_BASIC input ATTR_KEY(stage_in))
{
    return input.color;
}
