#pragma once
#include "VertexBuffer.h"

#include "VertexBufferLayout.h"

namespace Quack
{
	class VertexArray
	{
	private:
		unsigned int bufferId; // well it's not a buffer, but for convention we will it there :D  
	
	public:
		VertexArray();
		~VertexArray();

		void Bind();
		void Unbind();
		void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
	};
}
