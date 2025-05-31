#define HS_SHADER_FILE
#define HS_BASIC_SHADER
#include "../BuiltInMaterialLayout.h"

FSINPUT_BASIC VSENTRY_BASIC(
    VSINPUT_BASIC input)
{
    FSINPUT_BASIC output = (FSINPUT_BASIC)0;
    output.color = input.color;
    output.pos = input.pos;

    return output;
}

float4 FSENTRY_BASIC(FSINPUT_BASIC input) : SV_COLOR 
{
    return input.color;
}