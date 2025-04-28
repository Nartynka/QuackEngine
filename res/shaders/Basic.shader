#shader vertex
#version 460 core

layout(location = 0) in vec4 position;

uniform vec4 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 ourColor;

void main()
{
	gl_Position = projection * view * model * position;
	ourColor = color;
};

#shader fragment
#version 460 core

layout(location = 0) out vec4 color;

in vec4 ourColor;

void main()
{
	color = ourColor;
};
