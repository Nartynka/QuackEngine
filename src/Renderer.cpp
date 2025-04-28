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

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

}
