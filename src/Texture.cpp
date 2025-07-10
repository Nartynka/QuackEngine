#include "Texture.h"

#include <GL\glew.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Log.h"

namespace Quack
{
	unsigned int Texture::LoadFromFile(const char* path)
	{
		stbi_set_flip_vertically_on_load(1);
		
		int width, height, nrChannels;
		unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
		
		unsigned int id;
		
		if (data)
		{
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_2D, id);
			
			// @TODO: Add a feature to pass an argument if the texture has an alpha channel or check the file extension png / jpg
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(data);

			QUACK_GOOD("Texture {} with id {} loaded properly!", path, id);
			return id;
		}


		QUACK_ERROR("Failed to load texture {}", path);
		return 0;
	}

}