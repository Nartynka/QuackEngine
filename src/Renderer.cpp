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
		glEnable(GL_DEPTH_TEST);
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

	// Draw outlines of a cube for debug purpose
	void Renderer::DrawOutline(const VertexArray& va, const Shader& shader) const
	{
		shader.Bind();
		va.Bind();

		static int indices[] = {
			0, 1,  1, 2,  2, 3,  3, 0, // Front Face
			4, 5,  5, 6,  6, 7,  7, 4, // Back Face
			0, 4,  1, 5,  2, 6,  3, 7  // Side connecting lines
		};

		static IndexBuffer ib = {indices, 24};
		ib.Bind();

		glLineWidth(2.5f);

		glDrawElements(GL_LINES, 24,GL_UNSIGNED_INT, nullptr);

		glLineWidth(1.0f);
	}

}
