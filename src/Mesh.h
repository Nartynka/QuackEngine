#pragma once

#include <glm.hpp>
#include <vector>
#include <memory>
#include <string>

//#include "IndexBuffer.h"
//#include "VertexBuffer.h"
//#include "VertexArray.h"

namespace Quack
{
	class Shader;
	class VertexArray;
	class VertexBuffer;
	class IndexBuffer;

	struct Vertex
	{
		glm::vec3 position;
		//glm::vec3 normal;
		//glm::vec2 texCoords;
	};

	struct Texture
	{
		unsigned int id;
		std::string type; // in future maybe change it to enum or smth
		std::string path; // In future. store path to compare if the texture wasn't already loaded. 
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