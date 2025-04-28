#pragma once

namespace Quack
{
	class VertexArray;
	class VertexBuffer;
	class IndexBuffer;
	class Shader;

	class Renderer
	{
	private:
		Renderer();

		VertexArray* va;
		VertexBuffer* vb;
		IndexBuffer* ib;
		Shader* shader;
	public:
		~Renderer();

		static Renderer* Create();
		void Draw();

	};
}
