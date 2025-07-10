#include "Mesh.h"

#include <GL\glew.h>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Log.h"

namespace Quack
{
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<int>& indices, const std::vector<Texture>& textures)
		: vertices(vertices), indices(indices), textures(textures)
	{
		SetupMesh();
	}

	// This here because types for the buffers have to be complete for the unique_ptr
	Mesh::Mesh(Mesh&&) noexcept = default;
	
	Mesh::~Mesh() = default;

	void Mesh::SetupMesh()
	{
		// Create OpenGL objects
		vao = std::make_unique<VertexArray>();
		vao->Bind();
		vbo = std::make_unique<VertexBuffer>(vertices.data(), (unsigned int)(vertices.size() * sizeof(Vertex)));
		ibo = std::make_unique<IndexBuffer>(indices.data(), (unsigned int)indices.size());

		VertexBufferLayout layout;
		layout.AddElement(3); // Position
		layout.AddElement(2); // Texture coords

		vao->AddBuffer(*vbo, layout);
	}

	void Mesh::Draw(Shader& shader) const
	{
		if (!vao || indices.empty())
		{
			QUACK_ERROR("Vertex Array is invalid or indices vector is empty!");
			return;
		}

		// @TODO: multiple textures for mesh
		glActiveTexture(GL_TEXTURE0);
		if (!textures.empty())
			glBindTexture(GL_TEXTURE_2D, textures[0].id);
		else
			shader.SetUniform4f("inColor", 0.5, 0.0, 0.5);
		// @TODO: think about who should make the draw call ?
		vao->Bind();
		glDrawElements(GL_TRIANGLES, ibo->GetCount(), GL_UNSIGNED_INT, nullptr);
	}

}
