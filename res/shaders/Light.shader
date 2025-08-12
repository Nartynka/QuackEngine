#shader vertex
#version 460 core

layout(location = 0) in vec3 position;

uniform mat4 MVP;

void main()
{
   gl_Position = MVP * vec4(position, 1.0);
};

#shader fragment
#version 460 core

out vec4 color;

void main()
{
    color = vec4(1.0);
};