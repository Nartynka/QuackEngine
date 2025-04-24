#pragma once
#include <vector>

namespace Quack
{
	struct VertexAttribLayout
	{
		unsigned int id;
		unsigned int count;
		void* offset;
	};

	class VertexBufferLayout
	{
	private:
		unsigned int stride = 0;
		std::vector<VertexAttribLayout> elements;

	public:
		// We assume that every attrib will be of type float so we only need the count, the rest we can calculate
		// maybe in future this will change to accept different types
		void AddElement(unsigned int count)
		{
			elements.push_back({ (unsigned int)elements.size(), count, 
				elements.empty() ? nullptr : (void*)(elements.back().count * sizeof(float)) });
			stride += count * sizeof(float);
		}

		inline unsigned int GetStride() const { return stride; }
		inline const std::vector<VertexAttribLayout> GetElements() const { return elements; }

	};

}
