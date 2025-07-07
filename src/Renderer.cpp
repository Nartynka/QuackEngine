#include "Renderer.h"

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

#include <GL/glew.h>

namespace Quack
{
	Renderer::Renderer()
	{
		//glEnable(GL_DEPTH_TEST);
	}

	Renderer* Renderer::Create()
	{
		return new Renderer();
	}

	Renderer::~Renderer()
	{
	}


	void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
	{
		shader.Bind();
		va.Bind();
		ib.Bind();

		glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
	}

}
