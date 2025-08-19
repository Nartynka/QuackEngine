#pragma once

#include <vector>

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
		static void DrawNotIndexed(const VertexArray& vao, size_t count, const Shader& shader);
		static void DrawOutline(const VertexArray& vao, const Shader& shader);
		static void DrawMesh(const VertexArray& vao, const IndexBuffer& ibo, const std::vector<Texture>& textures, Shader& shader);
	};
}
