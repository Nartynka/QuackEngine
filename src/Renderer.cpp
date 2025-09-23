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
	Shader* Renderer::linesShader = nullptr;

	void Renderer::Init()
	{
		glEnable(GL_DEPTH_TEST);
		glPointSize(10.f);
		// this buffer should be deleted in some Terminate/Shutdown method or smth
		glGenBuffers(1, &lineBufferId);

		// this also should be deleted in some destructor function
		linesShader = new Shader("res/shaders/Lines.shader");
	}

	void Renderer::Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader)
	{
		shader.Bind();
		vao.Bind();
		//ibo.Bind();

		glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	void Renderer::DrawNotIndexed(const VertexArray& vao, unsigned int count, const Shader& shader)
	{
		shader.Bind();
		vao.Bind();

		glDrawArrays(GL_TRIANGLES, 0, count);
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

		glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);
	}

	// @TODO: remove passing ibo and just pass ibo.GetCount() because we don't have to bind ibo everytime
	// For debug

	void Renderer::DrawOutline(const VertexArray& vao, const IndexBuffer& ibo, glm::mat4 model, glm::vec3 color /*= glm::vec3(0.f, 0.5f, 1.f)*/)
	{
		linesShader->Bind();
		vao.Bind();
		//ibo.Bind();

		linesShader->SetUniform3fv("inColor", glm::value_ptr(color));
		linesShader->SetUniform4fm("model", glm::value_ptr(model));

		glLineWidth(2.5f);

		glDrawElements(GL_LINES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr);

		glLineWidth(1.0f);
	}

	void Renderer::DrawLine(glm::vec3 start, glm::vec3 end, glm::vec3 color /*= glm::vec3(0.f, 1.f, 0.f)*/)
	{
		linesShader->Bind();
		
		linesShader->SetUniform3fv("inColor", glm::value_ptr(color));
		linesShader->SetUniform4fm("model", glm::value_ptr(glm::mat4(1.0f)));

		glm::vec3 data[] = {start, end};
		
		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, lineBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		
		glLineWidth(4.f);
		glDrawArrays(GL_LINES, 0, 2);

		linesShader->SetUniform3f("inColor", 1.f, 1.f, 1.f);
		glDrawArrays(GL_POINTS, 0, 1);

		linesShader->SetUniform3f("inColor", 0.f, 1.f, 0.f);
		glDrawArrays(GL_POINTS, 1, 1);

		glLineWidth(1.0f);
	}

	void Renderer::DrawPoint(glm::vec3 point, glm::vec3 color /*= glm::vec3(0.f, 1.f, 1.f)*/)
	{
		linesShader->Bind();

		linesShader->SetUniform3fv("inColor", glm::value_ptr(color));
		linesShader->SetUniform4fm("model", glm::value_ptr(glm::mat4(1.0f)));

		glBindVertexArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, lineBufferId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), &point, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		
		glDrawArrays(GL_POINTS, 0, 1);
	}

}
