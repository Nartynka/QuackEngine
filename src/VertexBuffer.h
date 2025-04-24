#pragma once

namespace Quack
{
	class VertexBuffer
	{
	private:
		unsigned int bufferId = 0;

	public:
		// size in bytes
		VertexBuffer(const void* data, unsigned int size);
		~VertexBuffer();
		
		void Bind() const;
		void Unbind() const;
	};
}
