#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 67 0
layout(location = 0)
out vec4 entryPointParam_Basic_vertex_main_color_0;


#line 67
layout(location = 0)
in vec4 input_pos_0;


#line 67
layout(location = 1)
in vec4 input_color_0;


#line 68
struct Basic_fragment_input_0
{
    vec4 pos_0;
    vec4 color_0;
};


#line 69
void main()
{

#line 9 1
    const vec4 _S1 = vec4(0.0, 0.0, 0.0, 0.0);

#line 9
    Basic_fragment_input_0 output_0;

#line 9
    output_0.pos_0 = _S1;

#line 9
    output_0.color_0 = _S1;
    output_0.color_0 = input_color_0;
    output_0.pos_0 = input_pos_0;

    Basic_fragment_input_0 _S2 = output_0;

#line 13
    gl_Position = output_0.pos_0;

#line 13
    entryPointParam_Basic_vertex_main_color_0 = _S2.color_0;

#line 13
    return;
}

