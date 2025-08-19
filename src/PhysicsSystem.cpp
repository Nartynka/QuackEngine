#include "Systems.h"

#include "Scene.h"

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

namespace Quack
{
	// @TODO: should collision and physics be one component / system?

	void Move(const std::shared_ptr<Scene> scene, float dt)
	{
		auto& registry = scene->GetRegistry();
		auto physicsView = registry.view<PhysicsComponent, TransformComponent>();

		// entity is just a uint32_t so no need for a const reference
		for (auto entity : physicsView)
		{
			auto& [physics, transform] = physicsView.get(entity);
			physics.velocity += physics.acceleration * dt;
			transform.transform = glm::translate(transform.transform, physics.velocity * dt);
		}
	}

	void CheckCollision(const std::shared_ptr<Scene> scene, float dt, glm::vec3 floorPos, glm::vec3 floorHalfSize)
	{
		auto& registry = scene->GetRegistry();
		auto collisionView = registry.view<CollisionComponent, TransformComponent, PhysicsComponent>();

		static glm::vec3 minFloor;
		minFloor.x = floorPos.x - floorHalfSize.x;
		minFloor.y = floorPos.y - floorHalfSize.y;
		minFloor.z = floorPos.z - floorHalfSize.z;

		static glm::vec3 maxFloor;
		maxFloor.x = floorPos.x + floorHalfSize.x;
		maxFloor.y = floorPos.y + floorHalfSize.y;
		maxFloor.z = floorPos.z + floorHalfSize.z;


		for (auto entity : collisionView)
		{
			auto& [collision, transform, physics] = collisionView.get(entity);

			// Check collision with floor
			glm::vec3 minEntity;
			minEntity.x = transform.transform[3][0] - collision.halfSize.x;
			minEntity.y = transform.transform[3][1] - collision.halfSize.y;
			minEntity.z = transform.transform[3][2] - collision.halfSize.z;

			glm::vec3 maxEntity;
			maxEntity.x = transform.transform[3][0] + collision.halfSize.x;
			maxEntity.y = transform.transform[3][1] + collision.halfSize.y;
			maxEntity.z = transform.transform[3][2] + collision.halfSize.z;

			if ((minEntity.x <= maxFloor.x && maxEntity.x >= minFloor.x) &&
				(minEntity.y <= maxFloor.y && maxEntity.y >= minFloor.y) &&
				(minEntity.z <= maxFloor.z && maxEntity.z >= minFloor.z))
			{
				transform.transform = glm::translate(transform.transform, -(physics.velocity * dt));

				physics.velocity = glm::vec3(0.f);
				//physics.acceleration = glm::vec3(0.f); 'Oh gravity, thou art a heartless bitch' - Jim Parsons
			}
			else
			{
				// Check collision with everything else
				for (auto entity2 : collisionView)
				{
					if (entity == entity2)
						continue;

					auto& [collision2, transform2, physics2] = collisionView.get(entity2);

					glm::vec3 minEntity2;
					minEntity2.x = transform2.transform[3][0] - collision2.halfSize.x;
					minEntity2.y = transform2.transform[3][1] - collision2.halfSize.y;
					minEntity2.z = transform2.transform[3][2] - collision2.halfSize.z;

					glm::vec3 maxEntity2;
					maxEntity2.x = transform2.transform[3][0] + collision2.halfSize.x;
					maxEntity2.y = transform2.transform[3][1] + collision2.halfSize.y;
					maxEntity2.z = transform2.transform[3][2] + collision2.halfSize.z;

					if ((minEntity.x <= maxEntity2.x && maxEntity.x >= minEntity2.x) &&
						(minEntity.y <= maxEntity2.y && maxEntity.y >= minEntity2.y) &&
						(minEntity.z <= maxEntity2.z && maxEntity.z >= minEntity2.z))
					{
						transform.transform = glm::translate(transform.transform, -(physics.velocity * dt));

						physics.velocity = glm::vec3(0.f);

						break;
					}
				}
			}
		}
	}
}
