#pragma once

#include <glm.hpp>

#include "Shapes.h"
#include "Model.h"

namespace Quack
{
	struct TransformComponent
	{
		glm::mat4 transform = glm::mat4{ 1.0f };

		TransformComponent() = default;
		TransformComponent(glm::mat4 transform)
			: transform(transform) {}
	};

	struct PhysicsComponent
	{
		glm::vec3 velocity;
		glm::vec3 acceleration = glm::vec3(0.0f);

		float mass;

		PhysicsComponent(glm::vec3 velocity, glm::vec3 acceleration, float mass = 0.f)
			: velocity(velocity), acceleration(acceleration), mass(mass) {}
		PhysicsComponent(glm::vec3 velocity)
			: velocity(velocity), acceleration(0), mass(0) {}
	};

	struct CollisionComponent
	{
		// width, height, depth
		glm::vec3 halfSize;
		Shape* shape; // Only for drawing / debug

		CollisionComponent(glm::vec3 halfSize)
			: halfSize(halfSize)
		{
			shape = new Cube(halfSize);
		}

		~CollisionComponent()
		{
			delete shape;
		}
	};

	struct ShapeComponent
	{
		Shape* shape;
		ShapeComponent(Shape* shape)
			: shape(shape) {}

		~ShapeComponent()
		{
			delete shape;
		}
	};

	struct ModelComponent
	{
		Model* model;
		ModelComponent(Model* model)
			: model(model) {}

		~ModelComponent() = default; // ModelLibrary owns the model and deletes it

	};
}
