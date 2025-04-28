#pragma once

#include <string>

namespace Quack
{
	struct ShaderSource
	{
		std::string vertexShader;
		std::string fragmentShader;
	};

	class Shader
	{
	public:
		Shader(const char* filepath);
		~Shader();

		unsigned int shaderId;

		void Bind() const;
		void Unbind() const;

		// later maybe change to glm vec4 or something
		void SetUniform4f(const char* name, float v0, float v1, float v2, float v3 = 1.0f);

	private:
		ShaderSource ParseShader(const char* filepath);
		unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
		unsigned int CompileShader(unsigned int type, const std::string& source);

		int GetUniformLocation(const char* name);
	};
}
