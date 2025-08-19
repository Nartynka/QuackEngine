#pragma once

#include <glm.hpp>

#include "Shapes.h"
#include "Model.h"
#include "Log.h"

namespace Quack
{
	struct TransformComponent
	{
		glm::mat4 transform = glm::mat4{ 1.0f };

		TransformComponent() = default;
		TransformComponent(glm::mat4 transform)
			: transform(transform) {}
	};

	struct PhysicsComponent // rigid body component?
	{
		glm::vec3 velocity;

		glm::vec3 position, oldPosition;
		glm::vec3 forces;

		float mass = 1.0f;
		float bounce = 0.7f; // coefficient of restitution, how much energy is kept when entity bounces off a surface

		// Shared across entities
		glm::vec3 gravity = glm::vec3(0.0f, -9.82f, 0.0f);
		float friction = 0.98f;

		//PhysicsComponent(glm::vec3 velocity, glm::vec3 acceleration, float mass = 0.f)
		//	: velocity(velocity), acceleration(acceleration), mass(mass) {}
		//PhysicsComponent(glm::vec3 velocity)
		//	: velocity(velocity), mass(0) {}
		
		PhysicsComponent(glm::vec3 position, float mass = 1.0f)
			: position(position), mass(mass) {}

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
