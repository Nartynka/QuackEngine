#include "Shader.h"
#include "Log.h"

#include <GL/glew.h>

#include <string>
#include <fstream>
#include <sstream>
#include <cassert>

namespace Quack
{
	Shader::Shader(const char* filepath)
	{
		ShaderSource source = ParseShader(filepath);
		shaderId = CreateShader(source.vertexShader, source.fragmentShader);
	}

	Shader::~Shader()
	{
		glDeleteShader(shaderId);
	}

	void Shader::Bind() const
	{
		glUseProgram(shaderId);
	}

	void Shader::Unbind() const
	{
		glUseProgram(0);
	}

	ShaderSource Shader::ParseShader(const char* filepath)
	{
		std::ifstream file(filepath);

		assert(file.good() && "Error opening file!\n");

		enum class ShaderType
		{
			NONE = -1, VERTEX = 0, FRAGMENT = 1
		};

		std::string line;
		std::stringstream ss[2];
		ShaderType type = ShaderType::NONE;

		while (getline(file, line))
		{
			// if line is a start of a shader
			if (line.find("#shader") != std::string::npos)
			{
				if (line.find("vertex") != std::string::npos)
				{
					type = ShaderType::VERTEX;
				}
				else if (line.find("fragment") != std::string::npos)
				{
					type = ShaderType::FRAGMENT;

				}
			}
			else // if line is part of a shader
			{
				ss[(int)type] << line << "\n";
			}

		}

		file.close();

		return { ss[0].str(), ss[1].str()};
	}

	unsigned int  Shader::CompileShader(unsigned int type, const std::string& source)
	{
		unsigned int id = glCreateShader(type);
		const char* src = source.c_str();
		glShaderSource(id, 1, &src, nullptr);
		glCompileShader(id);

		int result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);

		if (result == GL_FALSE)
		{
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

			char* message = (char*)malloc(length * sizeof(char));
			glGetShaderInfoLog(id, length, &length, message);

			QUACK_ERROR("Failed to compile {} shader", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
			QUACK_ERROR(message);

			glDeleteShader(id);
			return 0;
		}

		return id;
	}

	unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
	{
		unsigned int program = glCreateProgram();

		// Compile two shader into one
		unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
		unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

		// "Link" shaders
		glAttachShader(program, vs);
		glAttachShader(program, fs);

		glLinkProgram(program);
		glValidateProgram(program);

		glDeleteShader(vs);
		glDeleteShader(fs);

		return program;
	}

	void Shader::SetUniform4f(const char* name, float v0, float v1, float v2, float v3)
	{
		glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
	}

	void Shader::SetUniform4fv(const char* name, const float* value)
	{
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value);
	}

	int Shader::GetUniformLocation(const char* name)
	{
		// Cashing uniforms? 
		int location = glGetUniformLocation(shaderId, name);

		// unused uniforms get stripped out so this should be changed to if statement in future
		assert(location != -1 && "Uniform not found!");

		return location;
	}

}
