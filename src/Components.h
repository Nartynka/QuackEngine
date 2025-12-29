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
		glm::vec3 angularVelocity;

		glm::vec3 position, oldPosition;
		glm::quat orientation; // rotation

		glm::vec3 forces;

		glm::mat3 invInertiaTensor; // body resistance to rotation

		float mass = 1.0f;
		float bounce = 0.7f; // coefficient of restitution, how much energy is kept when entity bounces off a surface

		// Shared across entities
		glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
		float friction = 0.98f; // linear damping?
		
		PhysicsComponent(glm::vec3 position, float mass = 1.0f, glm::vec3 halfSize = glm::vec3(1.f), float rotationAngle = 0.f, glm::vec3 rotationAxis = glm::vec3(0.f), float bounce = 0.7f, float friction = 0.98f)
			: position(position), mass(mass), bounce(bounce), friction(friction)
		{
			float angle = glm::radians(rotationAngle) / 2;
			orientation.x = rotationAxis.x * sin(angle);
			orientation.y = rotationAxis.y * sin(angle);
			orientation.z = rotationAxis.z * sin(angle);
			orientation.w = cos(angle);

			// I_xx = 1/12 * m * (h^2 + d^2)
			// I_yy = 1/12 * m * (w^2 + d^2)
			// I_zz = 1/12 * m * (w^2 + h^2)
			invInertiaTensor[0][0] = 1.f / (1.f / 12.f * mass * (halfSize.y * halfSize.y + halfSize.z * halfSize.z));
			invInertiaTensor[1][1] = 1.f / (1.f / 12.f * mass * (halfSize.x * halfSize.x + halfSize.z * halfSize.z));
			invInertiaTensor[2][2] = 1.f / (1.f / 12.f * mass * (halfSize.x * halfSize.x + halfSize.y * halfSize.y));
		}

		PhysicsComponent(glm::vec3 position, float mass = 1.0f, float radius = 1.f, float rotationAngle = 0.f, glm::vec3 rotationAxis = glm::vec3(0.f), float bounce = 0.7f, float friction = 0.98f)
			: position(position), mass(mass), bounce(bounce), friction(friction)
		{
			float angle = glm::radians(rotationAngle) / 2;
			orientation.x = rotationAxis.x * sin(angle);
			orientation.y = rotationAxis.y * sin(angle);
			orientation.z = rotationAxis.z * sin(angle);
			orientation.w = cos(angle);

			// I_diag = 2/5mr^2
			float invInertia = 1.f / (2.f / 5.f * mass * radius * radius);

			invInertiaTensor[0][0] = invInertia;
			invInertiaTensor[1][1] = invInertia;
			invInertiaTensor[2][2] = invInertia;
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
		float radius;
		Shape* shape; // Only for debug

		CollisionComponent(glm::vec3 halfSize)
			: halfSize(halfSize), radius(0.f)
		{
			shape = new DebugCube(halfSize);
		}

		CollisionComponent(float radius)
			: radius(radius)
		{
			shape = new DebugSphere(radius);
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
		glm::vec4 color;

		ShapeComponent(Shape* shape, glm::vec4 color = glm::vec4(1.f))
			: shape(shape), color(color) {}

		~ShapeComponent()
		{
			delete shape;
		}
	};

	struct ModelComponent
	{
		Model* model;
		glm::vec4 color;

		ModelComponent(Model* model, glm::vec4 color = glm::vec4(0.f))
			: model(model), color(color) {}

		~ModelComponent() = default; // ModelLibrary owns the model and deletes it

	};
}
