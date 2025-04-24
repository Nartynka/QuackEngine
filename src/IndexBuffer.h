#pragma once

namespace Quack
{
	class IndexBuffer
	{
	private:
		unsigned int bufferId;
		unsigned int count; // do i need to store it :thinking:

	public:
		// size in bytes
		IndexBuffer(const int* data, unsigned int count);
		~IndexBuffer();
		
		void Bind() const;
		void Unbind() const;

		inline unsigned int GetCount() const { return count; }
	};
}
