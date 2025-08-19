#pragma once

namespace Quack
{
	class VertexArray;
	class IndexBuffer;
	class Shader;

	class Renderer
	{
	public:		
		static void Init();

		static void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
		static void DrawOutline(const VertexArray& va, const Shader& shader);
	};
}
