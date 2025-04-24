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
		Shader();
		~Shader();

		static ShaderSource ParseShader(const char* filepath);
		static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

	private:
		static unsigned int CompileShader(unsigned int type, const std::string& source);
	};
}
