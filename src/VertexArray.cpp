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

	void VertexArray::Bind() const
	{
		glBindVertexArray(bufferId);
	}

	void VertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
	{
		Bind();
		vb.Bind();
		const auto& elements = layout.GetElements();

		for (int i = 0; i < elements.size(); i++)
		{
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, elements[i].count, GL_FLOAT, GL_FALSE, layout.GetStride(), (const void*)elements[i].offset);
		}
	}
}
