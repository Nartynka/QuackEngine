#pragma once

#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include "Shapes.h"
#include "Model.h"
#include "Log.h"

namespace Quack
{
	/// Physics component

	class Entity;

	struct RigidBodyComponent
	{
		glm::vec3 velocity = glm::vec3(0.f);
		glm::vec3 angularVelocity = glm::vec3(0.f);

		glm::vec3 forces;

		glm::mat3 invInertiaTensor; // body resistance to rotation

		float invMass = 1.0f;
		float bounce; // coefficient of restitution, how much energy is kept when entity collides with something

		glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);
		float friction; // damping, in future will implement proper Coulomb friction

		RigidBodyComponent()
			: invMass(0.f), invInertiaTensor(glm::mat3(0.f)), bounce(0.f), friction(0.98f)
		{
		}

		RigidBodyComponent(float mass, glm::vec3 halfSize, float bounce = 0.3f, float friction = 0.98f)
			: invMass(1.f / mass), bounce(bounce), friction(friction)
		{
			// I_xx = 1/12 * m * (h^2 + d^2)	
			// I_yy = 1/12 * m * (w^2 + d^2)
			// I_zz = 1/12 * m * (w^2 + h^2)

			invInertiaTensor[0][0] = 1.f / (1.f / 12.f * mass * (halfSize.y * halfSize.y + halfSize.z * halfSize.z));
			invInertiaTensor[1][1] = 1.f / (1.f / 12.f * mass * (halfSize.x * halfSize.x + halfSize.z * halfSize.z));
			invInertiaTensor[2][2] = 1.f / (1.f / 12.f * mass * (halfSize.x * halfSize.x + halfSize.y * halfSize.y));
		}

		RigidBodyComponent(float mass, float radius, float bounce = 0.3f, float friction = 0.98f)
			: invMass(1.f / mass), bounce(bounce), friction(friction)
		{
			// I_diag = 2/5mr^2
			float invInertia = 1.f / (2.f / 5.f * mass * radius * radius);

			invInertiaTensor[0][0] = invInertia;
			invInertiaTensor[1][1] = invInertia;
			invInertiaTensor[2][2] = invInertia;
		}
	};

	enum class ColliderType
	{
		Sphere,
		Cube // box
	};

	struct ColliderComponent
	{
		glm::vec3 halfSize;
		float radius;
		std::unique_ptr<Shape> shape; // Only for debug
		ColliderType type;

		ColliderComponent(glm::vec3 halfSize)
			: halfSize(halfSize), radius(0.f), type(ColliderType::Cube)
		{
			shape = std::make_unique<DebugCube>(halfSize);
		}

		ColliderComponent(float radius)
			: radius(radius), type(ColliderType::Sphere)
		{
			shape = std::make_unique<DebugSphere>(radius);
		}
	};

	struct TransformComponent
	{
		glm::vec3 position;
		glm::quat orientation; // rotation

		// @TODO: think if transform should be stored or just calculated when rendered
		glm::mat4 transform = glm::mat4{ 1.0f };

		TransformComponent() = default;

		TransformComponent(glm::vec3 position, float rotationAngle = 0.f, glm::vec3 rotationAxis = glm::vec3(0.f))
			: position(position)
		{
			float angle = glm::radians(rotationAngle) / 2;
			orientation.x = rotationAxis.x * sin(angle);
			orientation.y = rotationAxis.y * sin(angle);
			orientation.z = rotationAxis.z * sin(angle);
			orientation.w = cos(angle);
		}
	};



	//struct ElasticConstraintComponent
	//{
	//	//int idx_a = -1;
	//	//int idx_b = -1;

	//	// Store entity pointer or a int idx :thinking:
	//	Entity* entity_a = nullptr;
	//	Entity* entity_b = nullptr;

	//	float distance = 0.1f;
	//	float stiffness = 0.1f;

	//	ElasticConstraintComponent(Entity* e1, Entity* e2, float distance = 0.1f) 
	//		: entity_a(e1), entity_b(e2), distance(distance) {}
	//};



	/// Rendering components

	struct ShapeComponent
	{
		std::unique_ptr<Shape> shape;
		glm::vec4 color;

		ShapeComponent(Shape* shape, glm::vec4 color = glm::vec4(1.f))
			: shape(shape), color(color) {}
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
