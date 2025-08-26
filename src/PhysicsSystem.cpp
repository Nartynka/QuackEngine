#include "Systems.h"

#include "Scene.h"
#include "Components.h"

#include "Renderer.h" // for debug only

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

namespace Quack
{
	void ApplyForces(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto physicsView = registry.view<PhysicsComponent>();

		for (auto entity : physicsView)
		{
			auto& [physics] = physicsView.get(entity);

			physics.forces = physics.gravity * physics.mass; // For now only one force, gravity
		}
	}


	void Update(const std::shared_ptr<Scene> scene, float dt)
	{
		auto& registry = scene->GetRegistry();
		auto physicsView = registry.view<PhysicsComponent>();

		for (auto entity : physicsView)
		{
			auto& [physics] = physicsView.get(entity);

			physics.oldPosition = physics.position;
			glm::vec3 oldVelocity = physics.velocity;

			glm::vec3 acceleration = physics.forces * (1.0f / physics.mass); // forces * inverse of mass

			physics.velocity = physics.velocity * physics.friction + acceleration * dt;
			physics.position += (oldVelocity + physics.velocity) * 0.5f * dt;
		}
	}

	void UpdateTransform(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto movableView = registry.view<PhysicsComponent, TransformComponent>();

		for (auto entity : movableView)
		{
			auto& [physics, transform] = movableView.get(entity);
			transform.transform = glm::translate(glm::mat4(1.0f), physics.position);
			transform.transform *= glm::toMat4(physics.orientation);	
		}

		auto constraintView = registry.view<ConstraintComponent, TransformComponent>();

		for (auto entity : constraintView)
		{
			auto& [constraint, transform] = constraintView.get(entity);
			transform.transform = glm::translate(glm::mat4(1.0f), constraint.position);
			transform.transform *= glm::toMat4(constraint.orientation);
		}

	}

	// @TODO: do not pass constraintComponent, loop over every constraintComponent
	void SolveConstraint(const std::shared_ptr<Scene> scene, const ConstraintComponent* floor)
	{
		auto& registry = scene->GetRegistry();
		auto collisionView = registry.view<CollisionComponent, PhysicsComponent>();

		for (auto entity : collisionView)
		{
			auto& [collision, physics] = collisionView.get(entity);

			// @TODO: find better way to distinct if collision shape is sphere or cube
			if (collision.radius > 0.f)
			{
				// Sphere - OBB collision check

				// This is the same as the line bellow
				//glm::vec3 axisX = floor->orientation * glm::vec3(1.f, 0.f, 0.f);
				//glm::vec3 axisY = floor->orientation * glm::vec3(0.f, 1.f, 0.f);
				//glm::vec3 axisZ = floor->orientation * glm::vec3(0.f, 0.f, 1.f);

				glm::mat3 axes = glm::toMat3(floor->orientation) * glm::mat3(1.f);
				
				// vector from sphere to floor
				glm::vec3 d = physics.position - floor->position;
				
				// Find the closest point on OBB to the sphere center
				float distX = dot(d, axes[0]);
				float distY = dot(d, axes[1]);
				float distZ = dot(d, axes[2]);

				// Without clamping the point would the sphere center
				distX = glm::clamp(distX, -floor->halfSize.x, floor->halfSize.x);
				distY = glm::clamp(distY, -floor->halfSize.y, floor->halfSize.y);
				distZ = glm::clamp(distZ, -floor->halfSize.z, floor->halfSize.z);

				// dist- are in floor local space, in that local space they are not rotated so we have to rotate them
				// axes are our "portal"/"bridge" between floor local space and world space, they represent the floor rotated axes
				// - that's why we multiply dist- by corresponding axes
				glm::vec3 closestPoint = floor->position + distX * axes[0] + distY * axes[1] + distZ * axes[2];

				//Renderer::DrawPoint(closestPoint);
				
				glm::vec3 diff = physics.position - closestPoint;
				float distanceSquared = dot(diff, diff);

				if (distanceSquared <= collision.radius * collision.radius)
				{
					glm::vec3 normal = floor->orientation * glm::vec3(0.0f, 1.0f, 0.0f);
					
					physics.position = closestPoint + normal; // obviously this is wrong, will change in next commit to collision contact point
					
					glm::vec3 velocityNormal = normal * glm::dot(normal, physics.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
					glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor
					
					physics.velocity = velocityTangental - velocityNormal * physics.bounce;
					physics.velocity = physics.velocity.y < 0.21f ? glm::vec3(0.f) : physics.velocity; // to prevent jittering
				}
			}
			else
			{
				// AABB - AABB collision check
				static glm::vec3 minFloor;
				minFloor.x = floor->position.x - floor->halfSize.x;
				minFloor.y = floor->position.y - floor->halfSize.y;
				minFloor.z = floor->position.z - floor->halfSize.z;

				static glm::vec3 maxFloor;
				maxFloor.x = floor->position.x + floor->halfSize.x;
				maxFloor.y = floor->position.y + floor->halfSize.y;
				maxFloor.z = floor->position.z + floor->halfSize.z;


				glm::vec3 minEntity;
				minEntity.x = physics.position.x - collision.halfSize.x;
				minEntity.y = physics.position.y - collision.halfSize.y;
				minEntity.z = physics.position.z - collision.halfSize.z;

				glm::vec3 maxEntity;
				maxEntity.x = physics.position.x + collision.halfSize.x;
				maxEntity.y = physics.position.y + collision.halfSize.y;
				maxEntity.z = physics.position.z + collision.halfSize.z;


				if ((minEntity.x <= maxFloor.x && maxEntity.x >= minFloor.x) &&
					(minEntity.y <= maxFloor.y && maxEntity.y >= minFloor.y) &&
					(minEntity.z <= maxFloor.z && maxEntity.z >= minFloor.z))
				{
					// for every action, there is an equal and opposite reaction

					float penetration = maxFloor.y - minEntity.y;
					if (penetration > 0.0f)
					{
						physics.position.y += penetration;
					}

					glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);

					glm::vec3 velocityNormal = normal * glm::dot(normal, physics.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
					glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor

					physics.velocity = velocityTangental - velocityNormal * physics.bounce;
					physics.velocity = physics.velocity.y < 0.22f ? glm::vec3(0.f) : physics.velocity; // to prevent jittering
				}
			}
		}
	}


	//void Move(const std::shared_ptr<Scene> scene, float dt)
	//{
	//	auto& registry = scene->GetRegistry();
	//	auto physicsView = registry.view<PhysicsComponent, TransformComponent>();
	//
	//	// entity is just a uint32_t so no need for a const reference
	//	for (auto entity : physicsView)
	//	{
	//		auto& [physics, transform] = physicsView.get(entity);
	//		physics.velocity += physics.acceleration * dt;
	//		transform.transform = glm::translate(transform.transform, physics.velocity * dt);
	//	}
	//}

	//void CheckCollision(const std::shared_ptr<Scene> scene, float dt, glm::vec3 floorPos, glm::vec3 floorHalfSize)
	//{
	//	auto& registry = scene->GetRegistry();
	//	auto collisionView = registry.view<CollisionComponent, TransformComponent, PhysicsComponent>();

	//	static glm::vec3 minFloor;
	//	minFloor.x = floorPos.x - floorHalfSize.x;
	//	minFloor.y = floorPos.y - floorHalfSize.y;
	//	minFloor.z = floorPos.z - floorHalfSize.z;

	//	static glm::vec3 maxFloor;
	//	maxFloor.x = floorPos.x + floorHalfSize.x;
	//	maxFloor.y = floorPos.y + floorHalfSize.y;
	//	maxFloor.z = floorPos.z + floorHalfSize.z;

	//	 
	//	for (auto entity : collisionView)
	//	{
	//		auto& [collision, transform, physics] = collisionView.get(entity);

	//		// Check collision with floor
	//		glm::vec3 minEntity;
	//		minEntity.x = transform.transform[3][0] - collision.halfSize.x;
	//		minEntity.y = transform.transform[3][1] - collision.halfSize.y;
	//		minEntity.z = transform.transform[3][2] - collision.halfSize.z;

	//		glm::vec3 maxEntity;
	//		maxEntity.x = transform.transform[3][0] + collision.halfSize.x;
	//		maxEntity.y = transform.transform[3][1] + collision.halfSize.y;
	//		maxEntity.z = transform.transform[3][2] + collision.halfSize.z;

	//		if ((minEntity.x <= maxFloor.x && maxEntity.x >= minFloor.x) &&
	//			(minEntity.y <= maxFloor.y && maxEntity.y >= minFloor.y) &&
	//			(minEntity.z <= maxFloor.z && maxEntity.z >= minFloor.z))
	//		{
	//			transform.transform = glm::translate(transform.transform, -(physics.velocity * dt));

	//			physics.velocity = glm::vec3(0.f);
	//			//physics.acceleration = glm::vec3(0.f); 'Oh gravity, thou art a heartless bitch' - Jim Parsons
	//		}
	//		else
	//		{
	//			// Check collision with everything else
	//			for (auto entity2 : collisionView)
	//			{
	//				if (entity == entity2)
	//					continue;

	//				auto& [collision2, transform2, physics2] = collisionView.get(entity2);

	//				glm::vec3 minEntity2;
	//				minEntity2.x = transform2.transform[3][0] - collision2.halfSize.x;
	//				minEntity2.y = transform2.transform[3][1] - collision2.halfSize.y;
	//				minEntity2.z = transform2.transform[3][2] - collision2.halfSize.z;

	//				glm::vec3 maxEntity2;
	//				maxEntity2.x = transform2.transform[3][0] + collision2.halfSize.x;
	//				maxEntity2.y = transform2.transform[3][1] + collision2.halfSize.y;
	//				maxEntity2.z = transform2.transform[3][2] + collision2.halfSize.z;

	//				if ((minEntity.x <= maxEntity2.x && maxEntity.x >= minEntity2.x) &&
	//					(minEntity.y <= maxEntity2.y && maxEntity.y >= minEntity2.y) &&
	//					(minEntity.z <= maxEntity2.z && maxEntity.z >= minEntity2.z))
	//				{
	//					transform.transform = glm::translate(transform.transform, -(physics.velocity * dt));

	//					physics.velocity = glm::vec3(0.f);

	//					break;
	//				}
	//			}
	//		}
	//	}
	//}
}
