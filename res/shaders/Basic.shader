#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

uniform vec4 inColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 ourTexCoord;
out vec4 ourColor;
out vec3 debugColor;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	ourColor = inColor;
	ourTexCoord = texCoord;
	debugColor = vec3(texCoord, 0.0);
};

#shader fragment
#version 460 core

out vec4 color;

uniform sampler2D sampler;

in vec4 ourColor;
in vec2 ourTexCoord;
in vec3 debugColor;


void main()
{
	vec4 texColor = texture(sampler, ourTexCoord);
	color = mix(texColor.rgba, ourColor, ourColor.a);
	//color = texture(sampler, ourTexCoord) * ourColor;
	//color = vec4(debugColor, 1.0);
};
