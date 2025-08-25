#include "Renderer.h"

#include <GL/glew.h>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"

namespace Quack
{
	unsigned int Renderer::lineBufferId = -1; // why I can set uint to a negative number xD

	void Renderer::Init()
	{
		glEnable(GL_DEPTH_TEST);
		glPointSize(10.f);
		// this buffer should be deleted in some Terminate/Shutdown method or smth
		glGenBuffers(1, &lineBufferId);
	}

	void Renderer::Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader)
	{
		shader.Bind();
		vao.Bind();
		ibo.Bind();

		glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::DrawNotIndexed(const VertexArray& vao, unsigned int count, const Shader& shader)
	{
		shader.Bind();
		vao.Bind();

		glDrawArrays(GL_TRIANGLES, 0, count);
	}

	// Draw outlines for debug
	void Renderer::DrawOutline(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader)
	{
		shader.Bind();
		vao.Bind();

		glLineWidth(2.5f);

		glDrawElements(GL_LINES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);

		glLineWidth(1.0f);
	}

	void Renderer::DrawMesh(const VertexArray& vao, const IndexBuffer& ibo, const std::vector<Texture>& textures, Shader& shader)
	{
		shader.Bind();
		vao.Bind();

		for (int i = 0; i < textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}

		if (textures.empty())
			shader.SetUniform4f("inColor", 0.0f, 1.0f, 0.5f);

		glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::DrawLine(glm::vec3 start, glm::vec3 end, const Shader& shader)
	{
		shader.Bind();
		
		glm::mat4 model = glm::mat4(1.0f);
		
		shader.SetUniform4f("inColor", 1.f, 0.f, 0.f);
		shader.SetUniform4fv("model", glm::value_ptr(model));

		glm::vec3 data[] = {start, end};
		
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, lineBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		
		glLineWidth(5.f);
		glDrawArrays(GL_LINES, 0, 2);

		shader.SetUniform4f("inColor", 1.f, 1.f, 1.f);
		glDrawArrays(GL_POINTS, 0, 1);

		shader.SetUniform4f("inColor", 0.f, 1.f, 0.f);
		glDrawArrays(GL_POINTS, 1, 1);

		glLineWidth(1.0f);
	}

	void Renderer::DrawPoint(glm::vec3 point, const Shader& shader)
	{
		shader.Bind();

		glm::mat4 model = glm::mat4(1.0f);

		shader.SetUniform4f("inColor", 0.f, 1.f, 1.f);
		shader.SetUniform4fv("model", glm::value_ptr(model));

		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, lineBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &point, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		
		glDrawArrays(GL_POINTS, 0, 1);
	}

}
