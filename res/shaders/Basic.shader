#shader vertex
#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 ourTexCoord;

out vec3 normal;
out vec3 fragPos;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	ourTexCoord = texCoord;
	normal = mat3(transpose(inverse(model))) * vertexNormal; // inverse is very costly operation
	fragPos = vec3(model * vec4(position, 1.0));			 // vertex position to world space?
};


#shader fragment
#version 460 core

in vec2 ourTexCoord;
in vec3 normal;
in vec3 fragPos;

uniform sampler2D sampler;
uniform vec4 inColor;

uniform vec3 lightColor;
uniform vec3 lightPos;

out vec4 color;

void main()
{
	vec4 texColor = texture(sampler, ourTexCoord);
	vec4 fragColor = mix(texColor.rgba, inColor, inColor.a);

	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - fragPos);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * fragColor.rgb;
	color = vec4(result, 1.0);
};
