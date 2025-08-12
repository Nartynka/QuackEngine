#pragma once

#include <gtc\type_ptr.hpp>

namespace Quack
{
	class Shader;
	class Cube;

	class LightCube
	{
	public:
		LightCube();
		~LightCube();

		// @TODO: make private
		Shader* shader;
		Cube* shape;
		glm::vec3 position;
	};
}