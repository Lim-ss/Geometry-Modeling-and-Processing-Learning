#shader vertex
#version 330 core

//layout(location = 0) empty
layout(location = 1) in vec4 position;
layout(location = 2) in vec3 color;
layout(location = 3) in vec3 normal;

out vec3 v_color;
out vec3 v_position;
out vec3 v_normal;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * position;
    v_color = color;
};


#shader fragment
#version 330 core

layout(location = 0) out vec4 f_color;

in vec3 v_color;

uniform int u_Mode;

void main()
{
    if(u_Mode == 1)//È«°×
    {
        f_color = vec4(1.0f, 1.0f, 1.0f, 1.0);
    }
    else if(u_Mode == 2)//È«ºÚ
    {
        f_color = vec4(0.0f, 0.0f, 0.0f, 1.0);
    }
    else if(u_Mode == 3)//´¿É«
    {
        f_color = vec4(v_color, 1.0);
    }
};