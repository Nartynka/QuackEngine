#include "VertexArray.h"

#include <GL\glew.h>

namespace Quack
{
	VertexArray::VertexArray()
	{
		glGenVertexArrays(1, &bufferId);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &bufferId);
	}

	void VertexArray::Bind()
	{
		glBindVertexArray(bufferId);
	}

	void VertexArray::Unbind()
	{
		glBindVertexArray(0);
	}

	void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
	{
		Bind();
		vb.Bind();
		const auto& elements = layout.GetElements();
		for (const auto& e : elements)
		{
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, e.count, GL_FLOAT, GL_FALSE, layout.GetStride(), e.offset);
		}
	}
}
