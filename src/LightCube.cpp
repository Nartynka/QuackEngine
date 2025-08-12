#include "LightCube.h"

#include "Shader.h"
#include "Shapes.h"


namespace Quack
{
	LightCube::LightCube() : position(glm::vec3(-3.f, 2.f, -5.f))
	{
		shader = new Shader("res/shaders/Light.shader");
		shape = new Cube(glm::vec3(0.25f));
	}

	LightCube::~LightCube()
	{
		delete shader;
	}

}