#define HS_SHADER_FILE
#define HS_BASIC_SHADER
#include "../BuiltInMaterialLayout.h"

[shader("vertex")]
FSINPUT_BASIC VSENTRY_BASIC(
    VSINPUT_BASIC input)
{
    FSINPUT_BASIC output = (FSINPUT_BASIC)0;
    output.color = input.color;
    output.pos = input.pos;

    return output;
}

[shader("fragment")]
float4 FSENTRY_BASIC(FSINPUT_BASIC input) SM_OUT_TARGET
{
    return input.color;
}