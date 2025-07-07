#include "VertexBuffer.h"

#include <GL\glew.h>

#include "Assert.h"

namespace Quack
{
	VertexBuffer::VertexBuffer(const void* data, unsigned int size)
	{
		QUACK_ASSERT(data != nullptr, "Data for vertex buffer is a nullptr!!");

		glGenBuffers(1, &bufferId);
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	}

	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &bufferId);
	}

	void VertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	}

	void VertexBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

}
