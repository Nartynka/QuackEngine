#pragma once

#include <glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include "Texture.h"

namespace Quack
{
	class Shader;
	class VertexArray;
	class VertexBuffer;
	class IndexBuffer;

	struct Vertex
	{
		glm::vec3 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
	};

	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<int>& indices, const std::vector<Texture>& textures);
		~Mesh();

		void Draw(Shader& shader) const;
		
		// Explicitly delete copy operations
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		// This is some magic here that i don't understand. Idk why this is not generated automatically by compiler
		// @TODO: research why and how this works
		Mesh(Mesh&&) noexcept;

		//  render data
		std::unique_ptr<VertexArray> vao;
		std::unique_ptr<VertexBuffer> vbo;
		std::unique_ptr<IndexBuffer> ibo;
	private:
		std::vector<Vertex> vertices;
		std::vector<int> indices;
		std::vector<Texture> textures;
	
		void SetupMesh();
	};
}