#include <metal_stdlib>

using namespace metal;

struct VSInput_Basic {
    float4 pos [[attribute(0)]];
    float4 color [[attribute(1)]];
};

struct FSInput_Basic {
    float4 pos [[position]];
    float4 color;
};

vertex FSInput_Basic VertexShader_Basic(uint vertexID [[vertex_id]],
                                      const device VSInput_Basic* input [[buffer(0)]])
{
    FSInput_Basic output;
    
    output.pos = input[vertexID].pos;
    output.color = input[vertexID].color;
    
    return output;
}

fragment float4 FragmentShader_Basic(FSInput_Basic input [[stage_in]])
{
    return input.color;
}
