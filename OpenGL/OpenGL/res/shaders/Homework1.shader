#shader vertex
#version 330 core
        
layout(location = 0) in vec4 position;
layout(location = 1) in vec3 color;

out vec3 v_color;

uniform  mat4 u_MVP;

void main()
{
    v_color = color;
    gl_Position = u_MVP * position;
};


#shader fragment
#version 330 core

layout(location = 0) out vec4 f_color;

in vec3 v_color;

void main()
{
    f_color = vec4(v_color, 1.0);
};