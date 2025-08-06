#pragma once

#include <glm.hpp>

namespace Quack
{
	class Camera
	{
	public:
		Camera();
		// @TODO: change to private members
		float speed = 5.f;
		float rotationSpeed = 0.01f;

		glm::vec3 position = glm::vec3(0.f, 0.5f, 5.f);
		glm::vec3 front = glm::vec3(0.f, 0.f, -1.f);
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);

		float yaw = -90.f;
		float pitch = 0.f;

		float lastX = -1.f;
		float lastY = -1.f;

		void Update(float dt); // maybe process input would be a better name
	};
}