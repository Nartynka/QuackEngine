#pragma once

namespace Quack
{
	class VertexArray;
	class IndexBuffer;
	class Shader;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();

		void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;
		void DrawOutline(const VertexArray& va, const Shader& shader) const;
	};
}
