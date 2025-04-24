#pragma once

namespace Quack
{
	class VertexArray;
	class VertexBuffer;
	class IndexBuffer;

	class Renderer
	{
	private:
		Renderer();

		unsigned int shaderId;

		VertexArray* va;
		VertexBuffer* vb;
		IndexBuffer* ib;
	public:
		~Renderer();

		static Renderer* Create();
		void Draw();

	};
}
