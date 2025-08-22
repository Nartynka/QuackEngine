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

		void SetUniform4f(const char* name, float v0, float v1, float v2, float v3 = 1.0f) const;
		void SetUniform3f(const char* name, float v0, float v1, float v2) const;
		void SetUniform3fv(const char* name, const float* value) const;
		void SetUniform4fv(const char* name, const float* value) const;
		void SetUniformInt(const char* name, int value) const;

	private:
		ShaderSource ParseShader(const char* filepath);
		unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
		unsigned int CompileShader(unsigned int type, const std::string& source);

		int GetUniformLocation(const char* name) const;
	};
}
