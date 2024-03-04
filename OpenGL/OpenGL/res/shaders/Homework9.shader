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
uniform mat4 u_Model;

void main()
{
    gl_Position = u_MVP * position;
    v_color = color;
    v_position = vec3(u_Model * position);
    v_normal = normal;
};


#shader fragment
#version 330 core

layout(location = 0) out vec4 f_color;

in vec3 v_color;
in vec3 v_position;
in vec3 v_normal;

uniform vec3 u_LightPosition;
uniform vec3 u_LightColor;
uniform vec3 u_ViewPosition;

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 lightDiretion = normalize(u_LightPosition - v_position);


    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_LightColor;

    float diff = max(dot(normal, lightDiretion), 0.0);
    vec3 diffuse = diff * u_LightColor;

    float specularStrength = 0.5;
    vec3 viewDiretion = normalize(u_ViewPosition - v_position);
    vec3 reflectDiretion = reflect(-lightDiretion, normal);
    float spec = pow(max(dot(viewDiretion, reflectDiretion), 0.0), 32);
    vec3 specular = specularStrength * spec * u_LightColor;

    vec3 result = (ambient + diffuse + specular) * v_color;
    f_color = vec4(result, 1.0);
};