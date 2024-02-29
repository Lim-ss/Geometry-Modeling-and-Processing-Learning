#shader vertex
#version 330 core
        
layout(location = 1) in vec4 position;
//layout(location = 1) in vec3 normal;
//layout(location = 2) in vec2 texCoord;

uniform  mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * position;
};


#shader fragment
#version 330 core

layout(location = 0) out vec4 f_color;

void main()
{
    f_color = vec4(1.0, 1.0, 1.0, 1.0);
};