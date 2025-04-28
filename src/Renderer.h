#pragma once

namespace Quack
{
	class VertexArray;
	class IndexBuffer;
	class Shader;

	class Renderer
	{
	private:
		Renderer();

	public:
		~Renderer();

		static Renderer* Create();
		void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const;

	};
}
