#pragma once

#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include "Shapes.h"
#include "Model.h"
#include "Log.h"

namespace Quack
{
	/// Physics component

	struct PhysicsComponent // rigid body component?
	{
		glm::vec3 velocity;

		glm::vec3 position, oldPosition;
		glm::quat orientation; // rotation

		glm::vec3 forces;

		float mass = 1.0f;
		float bounce = 0.7f; // coefficient of restitution, how much energy is kept when entity bounces off a surface

		// Shared across entities
		glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
		float friction = 0.98f;
		
		PhysicsComponent(glm::vec3 position, float mass = 1.0f, float rotationAngle = 0.f, glm::vec3 rotationAxis = glm::vec3(0.f))
			: position(position), mass(mass) 
		{
			float angle = glm::radians(rotationAngle) / 2;
			orientation.x = rotationAxis.x * sin(angle);
			orientation.y = rotationAxis.y * sin(angle);
			orientation.z = rotationAxis.z * sin(angle);
			orientation.w = cos(angle);
		}

	};

	struct ConstraintComponent // Immovable physics & collision component?
	{
		// Maybe will refactor later
		// width, height, depth
		glm::vec3 halfSize;
		glm::vec3 position;
		glm::quat orientation;  // rotation

		ConstraintComponent(glm::vec3 position, glm::vec3 halfSize, float rotationAngle = 0.f, glm::vec3 rotationAxis = glm::vec3(0.f))
			: position(position), halfSize(halfSize)
		{
			float angle = glm::radians(rotationAngle) / 2;
			orientation.x = rotationAxis.x * sin(angle);
			orientation.y = rotationAxis.y * sin(angle);
			orientation.z = rotationAxis.z * sin(angle);
			orientation.w = cos(angle);
		}
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


	/// Rendering components

	struct TransformComponent
	{
		glm::mat4 transform = glm::mat4{ 1.0f };

		TransformComponent() = default;
		TransformComponent(glm::mat4 transform)
			: transform(transform) {
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
