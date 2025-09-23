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
uniform vec3 viewPos;

out vec4 color;

void main()
{
	vec4 texColor = texture(sampler, ourTexCoord);
	vec4 fragColor = mix(texColor.rgba, inColor, inColor.a);

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - fragPos);

	float ambientStrength = 0.2;
	vec3 ambient = ambientStrength * lightColor;

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	float specularStrength = 0.01;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * fragColor.rgb;
	color = vec4(result, 1.0);

	// normal debug
	//color = vec4(norm * 0.5 + 0.5, 1.0); // map [-1,1] to [0,1]
};
