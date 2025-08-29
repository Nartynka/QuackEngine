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

			// @TODO: think about better way to do it
			glm::vec3 scale;
			scale.x = glm::length(transform.transform[0]);
			scale.y = glm::length(transform.transform[1]);
			scale.z = glm::length(transform.transform[2]);

			transform.transform = glm::translate(glm::mat4(1.0f), physics.position);
			transform.transform *= glm::toMat4(physics.orientation);
			transform.transform = glm::scale(transform.transform, scale);
		}

		auto constraintView = registry.view<ConstraintComponent, TransformComponent>();

		for (auto entity : constraintView)
		{
			auto& [constraint, transform] = constraintView.get(entity);
			transform.transform = glm::translate(glm::mat4(1.0f), constraint.position);
			transform.transform *= glm::toMat4(constraint.orientation);
		}

	}

	void SolveConstraints(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto collisionView = registry.view<CollisionComponent, PhysicsComponent>();

		auto constraints = registry.view<ConstraintComponent>();

		for (auto entity : collisionView)
		{
			auto& [collision, physics] = collisionView.get(entity);

			for (auto constraint : constraints)
			{
				// each constraint component is a floor
				const auto& floor = constraints.get<ConstraintComponent>(constraint);

				// @TODO: find better way to distinct if collision shape is sphere or cube
				if (collision.radius > 0.f)
				{
					glm::mat3 axes = glm::toMat3(floor.orientation) * glm::mat3(1.f);

					// vector from sphere to floor
					glm::vec3 d = physics.position - floor.position;

					// Find the closest point on OBB to the sphere center
					float distX = dot(d, axes[0]);
					float distY = dot(d, axes[1]);
					float distZ = dot(d, axes[2]);

					// Without clamping the point would the sphere center
					distX = glm::clamp(distX, -floor.halfSize.x, floor.halfSize.x);
					distY = glm::clamp(distY, -floor.halfSize.y, floor.halfSize.y);
					distZ = glm::clamp(distZ, -floor.halfSize.z, floor.halfSize.z);

					// dist- are in floor local space, in that local space they are not rotated so we have to rotate them
					// axes are our "portal"/"bridge" between floor local space and world space, they represent the floor rotated axes
					// - that's why we multiply dist- by corresponding axes
					glm::vec3 closestPoint = floor.position + distX * axes[0] + distY * axes[1] + distZ * axes[2];

					//Renderer::DrawPoint(closestPoint);

					glm::vec3 diff = physics.position - closestPoint;
					float distanceSquared = dot(diff, diff); // distance from closes point to the sphere. length of diff without expensive sqrt

					if (distanceSquared <= collision.radius * collision.radius)
					{
						glm::vec3 collisionNormal = normalize(diff); // direction from OBB to sphere

						float penetration = collision.radius - sqrt(distanceSquared); // @TODO: can I omit the sqrt here? 

						physics.position += collisionNormal * penetration;

						glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
						glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor

						physics.velocity = velocityTangental - velocityNormal * physics.bounce;
						// This line prevents jittering when sphere bounces forever (on not oriented surface) but prevents the sphere from rolling down sloped/oriented surface
						//physics.velocity = physics.velocity.y < 0.21f ? glm::vec3(0.f) : physics.velocity; // to prevent jittering
					}
				}
				else
				{
					// AABB - AABB collision check
					static glm::vec3 minFloor;
					minFloor.x = floor.position.x - floor.halfSize.x;
					minFloor.y = floor.position.y - floor.halfSize.y;
					minFloor.z = floor.position.z - floor.halfSize.z;

					static glm::vec3 maxFloor;
					maxFloor.x = floor.position.x + floor.halfSize.x;
					maxFloor.y = floor.position.y + floor.halfSize.y;
					maxFloor.z = floor.position.z + floor.halfSize.z;


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
	}

	void SolveCollision(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto collisionView = registry.view<CollisionComponent, PhysicsComponent>();

		for (auto entity : collisionView)
		{
			auto& [collision, physics] = collisionView.get(entity);

			// @TODO: find better way to distinct if collision shape is sphere or cube
			// @TODO: make proper sphere - sphere collision
			if (collision.radius > 0.f)
			{
				for (auto entity2 : collisionView)
				{
					if (entity == entity2)
						continue;

					auto& [collision2, physics2] = collisionView.get(entity2);

					glm::vec3 diff = physics.position - physics2.position;
					float distanceSquared = dot(diff, diff);
					float radii = collision.radius + collision2.radius; // radiuses

					if (distanceSquared < radii * radii)
					{
						glm::vec3 collisionNormal = normalize(diff); // direction from sphere to sphere
						
						float penetration = (radii - sqrt(distanceSquared)) / 2.f; // @TODO: can I omit the sqrt here? 
						physics.position += collisionNormal * penetration;
						physics2.position += -collisionNormal * penetration;
						
						{
							glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
							glm::vec3 velocityTangental = physics.velocity - velocityNormal;		// parallel to the floor

							physics.velocity = velocityTangental - velocityNormal * physics.bounce;
						}
						{
							glm::vec3 velocityNormal = collisionNormal * glm::dot(collisionNormal, physics2.velocity); // perpendicular to the floor. It's not normalized so the name is a bit misleading
							glm::vec3 velocityTangental = physics2.velocity - velocityNormal;		// parallel to the floor

							physics2.velocity = velocityTangental - velocityNormal * physics2.bounce;
						}
					}
				}
			}
		}
	}
}
