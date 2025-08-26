#shader vertex
#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   gl_Position = projection * view * model * vec4(position, 1.0);
};

#shader fragment
#version 460 core

uniform vec3 inColor;

out vec4 color;

void main()
{
    color = vec4(inColor, 1.0);
};