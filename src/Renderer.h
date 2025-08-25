#pragma once

#include <vector>
#include <glm.hpp>

namespace Quack
{
	class VertexArray;
	class IndexBuffer;
	class Shader;
	class Model;
	class Texture;

	class Renderer
	{
	public:		
		static void Init();

		static void Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader);
		static void DrawNotIndexed(const VertexArray& vao, unsigned int count, const Shader& shader);
		static void DrawOutline(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader);
		static void DrawMesh(const VertexArray& vao, const IndexBuffer& ibo, const std::vector<Texture>& textures, Shader& shader);
		static void DrawLine(glm::vec3 start, glm::vec3 end, const Shader& shader);
		static void DrawPoint(glm::vec3 point, const Shader& shader);

	private:
		static unsigned int lineBufferId;
	};
}
