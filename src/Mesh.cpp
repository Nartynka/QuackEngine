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
		//layout.AddElement(3); // Normals
		//layout.AddElement(2); // Texture coords

		vao->AddBuffer(*vbo, layout);
	}

	void Mesh::Draw(Shader& shader) const
	{
		if (!vao || indices.empty())
		{
			QUACK_ERROR("Vertex Array is invalid or indices vector is empty!");
			return;
		}

		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		//for (int i = 0; i < textures.size(); i++)
		//{
		//	glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		//	// retrieve texture number (the N in diffuse_textureN)
		//	std::string number;
		//	std::string name = textures[i].type;
		//	if (name == "texture_diffuse")
		//		number = std::to_string(diffuseNr++);
		//	else if (name == "texture_specular")
		//		number = std::to_string(specularNr++);

		//	shader.SetUniformInt(("material." + name + number).c_str(), i);
		//	glBindTexture(GL_TEXTURE_2D, textures[i].id);
		//}
		//glActiveTexture(GL_TEXTURE0);

		// draw mesh. In future - the renderer class should make every draw call ?
		vao->Bind();

		glDrawElements(GL_TRIANGLES, ibo->GetCount(), GL_UNSIGNED_INT, nullptr);
		//vao->Unbind();
	}

}