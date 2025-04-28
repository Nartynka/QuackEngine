#shader vertex
#version 460 core

layout(location = 0) in vec4 position;

uniform vec4 color;

out vec4 ourColor;

void main()
{
	gl_Position = position;
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
