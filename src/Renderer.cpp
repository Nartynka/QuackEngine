#include "Renderer.h"

#include <GL/glew.h>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"

namespace Quack
{
	void Renderer::Init()
	{
		glEnable(GL_DEPTH_TEST);
	}

	void Renderer::Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader)
	{
		shader.Bind();
		vao.Bind();
		ibo.Bind();

		glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::DrawNotIndexed(const VertexArray& vao, size_t count, const Shader& shader)
	{
		shader.Bind();
		vao.Bind();

		glDrawArrays(GL_TRIANGLES, 0, count);
	}

	// Draw outlines of a cube for debug purpose
	void Renderer::DrawOutline(const VertexArray& vao, const Shader& shader)
	{
		shader.Bind();
		vao.Bind();

		static int indices[] = {
			0, 1,  1, 2,  2, 3,  3, 0, // Front Face
			4, 5,  5, 6,  6, 7,  7, 4, // Back Face
			0, 4,  1, 5,  2, 6,  3, 7  // Side connecting lines
		};

		static IndexBuffer ibo = {indices, 24};
		ibo.Bind();

		glLineWidth(2.5f);

		glDrawElements(GL_LINES, 24,GL_UNSIGNED_INT, nullptr);

		glLineWidth(1.0f);
	}

	void Renderer::DrawMesh(const VertexArray& vao, const IndexBuffer& ibo, const std::vector<Texture>& textures, Shader& shader)
	{
		for (int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		vao.Bind();
		glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
	}

}
