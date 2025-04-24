#include "Renderer.h"

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

#include <GL/glew.h>

namespace Quack
{
	static float vertices[] = {
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f,
	};

	static int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	Renderer::Renderer()
	{
		VertexBufferLayout layout;
		layout.AddElement(2);
		
		vb = new VertexBuffer(vertices, 6 * 2 * sizeof(float));
		va = new VertexArray();
		va->AddBuffer(*vb, layout);

		ib = new IndexBuffer(indices, 6);
		ib->Bind();

		ShaderSource source = Shader::ParseShader("D:/Projects/GameEngine/res/shaders/Basic.shader");
		shaderId = Shader::CreateShader(source.vertexShader, source.fragmentShader);
		glUseProgram(shaderId);
	}

	Renderer* Renderer::Create()
	{
		return new Renderer();
	}

	Renderer::~Renderer()
	{
		delete va;
		delete vb;
		delete ib;
	}


	void Renderer::Draw()
	{
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

}
