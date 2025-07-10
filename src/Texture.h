#pragma once

#include <string>

namespace Quack
{
	class Texture
	{
	public:
		// in future maybe this shouldn't be a class
		unsigned int id;
		std::string type; // @TODO: in future maybe change it to enum or something
		std::string path; // @TODO: Store path to compare if the texture wasn't already loaded. 

		static unsigned int LoadFromFile(const char* path);
	};
}