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

		shader = new Shader("res/shaders/Basic.shader");
		shader->Bind();
		shader->SetUniform4f("color", 0, 0.5f, 0.5f);
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
		delete shader;
	}


	void Renderer::Draw()
	{
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

}
