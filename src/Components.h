#pragma once

#include <glm.hpp>

namespace Quack
{
	struct TransformComponent
	{
		glm::mat4 transform = glm::mat4{ 1.0f };

		TransformComponent() = default;
		TransformComponent(glm::mat4 transform)
			: transform(transform) {}

		operator glm::mat4()& { return transform; }
	};

	struct PhysicsComponent
	{
		glm::vec3 velocity;
		glm::vec3 acceleration = glm::vec3(0.0f);

		float mass;

		PhysicsComponent(glm::vec3 velocity, glm::vec3 acceleration, float mass)
			: velocity(velocity), acceleration(acceleration), mass(mass) {}
		PhysicsComponent(glm::vec3 velocity)
			: velocity(velocity), acceleration(0), mass(0) {}
	};
}