#include "Scene.h"

#include <glm.hpp>

#include "ModelLibrary.h"

namespace Quack
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity()
	{
		return Entity{ registry.create(), this };
	}

	entt::registry& Scene::GetRegistry()
	{
		return registry;
	}

	int Scene::GetEntityCount()
	{
		return (int)registry.storage<entt::entity>().free_list();
	}

	void Scene::ClearEntities()
	{
		registry.clear();
	}

	/// == Spawning scenes ==

	void Scene::SpawnFloorScene()
	{
		glm::vec3 floorHalfSize = glm::vec3(3.5f, 0.1f, 4.5f);
		glm::vec4 floorColor = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);

		Entity floor = CreateEntity();
		glm::vec3 floorPos = glm::vec3(0.f, -floorHalfSize.y, -5.f);
		floor.AddComponent<TransformComponent>(floorPos);
		floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		floor.AddComponent<ColliderComponent>(floorHalfSize);
	}

	void Scene::SpawnTestScene()
	{
		{ // first falling cube
			Entity entity = CreateEntity();
			glm::vec3 halfSize = glm::vec3(0.5f);
			glm::vec3 position = glm::vec3(0.f, 2.f, -5.f);
			entity.AddComponent<TransformComponent>(position);
			auto& rigidBodyComp = entity.AddComponent<RigidBodyComponent>(10.f, glm::vec3(0.5f));
			entity.AddComponent<ColliderComponent>(halfSize);
			entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(1.f, 0.f, 1.f, 1.f));
		}
		{ // first "static" cube (rigid of big mass)
			Entity entity = CreateEntity();
			glm::vec3 position = glm::vec3(0.f, 1.f, -5.f);
			entity.AddComponent<TransformComponent>(position, 0.f, glm::vec3(0.f, 0.f, 1.f));
			auto& rigidBodyComp = entity.AddComponent<RigidBodyComponent>(100.f, glm::vec3(0.5f));
			entity.AddComponent<ColliderComponent>(glm::vec3(0.5f));
			entity.AddComponent<ShapeComponent>(new NormalCube(), glm::vec4(0.5f, 1.0f, 0.5f, 1.f));
		}
		{ // second static cube, without rigid body
			Entity entity = CreateEntity();
			glm::vec3 position = glm::vec3(0.f, 0.f, -5.f);
			glm::vec3 halfSize = glm::vec3(0.5f);
			entity.AddComponent<TransformComponent>(position, 0.f, glm::vec3(0.f, 0.f, 1.f));
			entity.AddComponent<ColliderComponent>(halfSize);
			entity.AddComponent<ShapeComponent>(new NormalCube(halfSize), glm::vec4(0.0f, 0.5f, 0.5f, 1.0f));
		}

		{ // duck model
			Entity entity = CreateEntity();
			glm::vec3 position = glm::vec3(2.f, 0.f, -5.f);
			entity.AddComponent<TransformComponent>(position, 0.f, glm::vec3(0.f, 0.f, 1.f));
			entity.AddComponent<ColliderComponent>(glm::vec3(0.7f));
			entity.AddComponent<ModelComponent>(ModelLibrary::duck.get());
			auto& rigidBodyComp = entity.AddComponent<RigidBodyComponent>(1.f, glm::vec3(0.7f));
			rigidBodyComp.gravity = glm::vec3(0.f);
		}
		{ // second falling sphere
			Entity entity = CreateEntity();
			glm::vec3 position = glm::vec3(2.0f, 5.f, -6.f);
			float radius = 1.f;
			entity.AddComponent<TransformComponent>(position);
			auto& rigidBodyComp = entity.AddComponent<RigidBodyComponent>(10.f, radius);
			entity.AddComponent<ColliderComponent>(radius);
			entity.AddComponent<ShapeComponent>(new Sphere(radius), glm::vec4(1.f, 0.f, 1.f, 1.f));
		}
	}

	void Scene::SpawnTiltedFloorsScene()
	{
		glm::vec3 floorHalfSize = glm::vec3(3.5f, 0.1f, 4.5f);
		glm::vec4 floorColor = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);
		glm::vec3 floorPos = glm::vec3(2.f, 0.f, -5.f);
		float rotation = 40.f;

		for (int i = 0; i < 8; i++)
		{
			Entity floor = CreateEntity();
			floor.AddComponent<TransformComponent>(floorPos, rotation, glm::vec3(0.f, 0.f, 1.f));
			floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
			floor.AddComponent<ColliderComponent>(floorHalfSize);

			floorPos.x += 2;
			floorPos.x *= -1;
			floorPos.y -= 4;
			rotation *= -1;
		}
	}

	void Scene::SpawnCribbingTower()
	{
		glm::vec3 halfSize = glm::vec3(0.2f, 0.2f, 2.f);
		glm::vec4 cubeColor = glm::vec4(0.1f, 0.5f, 0.7f, 1.f);

		glm::vec3 frontPosition = glm::vec3(-1.f, halfSize.y, -5.f);
		glm::vec3 sidePosition = glm::vec3(frontPosition.x + halfSize.z * 0.5f, frontPosition.y + halfSize.y * 2, frontPosition.z + halfSize.z * 0.5f);
		int sign = 1;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				Entity front = CreateEntity();
				front.AddComponent<TransformComponent>(frontPosition);
				auto& r = front.AddComponent<RigidBodyComponent>(1.f, halfSize);
				r.gravity = glm::vec3(0.f);
				front.AddComponent<ColliderComponent>(halfSize);
				front.AddComponent<ShapeComponent>(new NormalCube(halfSize), cubeColor);
				frontPosition.x += halfSize.z * sign;
				sign *= -1;

				Entity side = CreateEntity();
				side.AddComponent<TransformComponent>(sidePosition, 90.f, glm::vec3(0.f, 1.f, 0.f));
				auto& r2 = side.AddComponent<RigidBodyComponent>(1.f, halfSize);
				r2.gravity = glm::vec3(0.f);
				side.AddComponent<ColliderComponent>(halfSize);
				side.AddComponent<ShapeComponent>(new NormalCube(halfSize), cubeColor);
				sidePosition.z = frontPosition.z + halfSize.z * 0.5f * sign;
			}
			frontPosition.y += halfSize.y * 4;
			sidePosition.y += halfSize.y * 4;
		}
	}

	/// == Spawning shape stacks ==

	void Scene::SpawnStackOfCubes(int* size, float* pos)
	{
		float z = pos[2];
		glm::vec4 cubeColor = glm::vec4(0.0f, 1.0f, 0.5f, 1.f);
		for (int i = 0; i < size[0]; i++)
		{
			float y = pos[1];
			for (int j = 0; j < size[1]; j++)
			{
				float x = pos[0];
				for (int k = 0; k < size[2]; k++)
				{
					Entity entity = CreateEntity();
					glm::vec3 position = glm::vec3(x, y, z);
					entity.AddComponent<TransformComponent>(position);
					entity.AddComponent<RigidBodyComponent>(1.f, glm::vec3(0.5f));
					entity.AddComponent<ColliderComponent>(glm::vec3(0.5f));
					entity.AddComponent<ShapeComponent>(new NormalCube(glm::vec3(0.5f)), cubeColor);
					x += 1.1f;
				}
				y += 1.1f;
			}
			z -= 1.1f;
		}
	}

	void Scene::SpawnStackOfSpheres(int* size, float* pos)
	{
		float z = pos[2];
		glm::vec4 sphereColor = glm::vec4(0.0f, 1.0f, 0.5f, 1.f);
		std::vector<glm::vec4> sphereColors = { glm::vec4(0.8f, 0.f, 1.f, 1.f), glm::vec4(0.f, 0.8f, 1.f, 1.f), glm::vec4(0.0f, 1.0f, 0.5f, 1.f), glm::vec4(1.f, 0.8f, 0.f, 1.f),  glm::vec4(1.f, 0.2f, 0.5f, 1.f) };
		float radius = 0.5f;
		for (int i = 0; i < size[0]; i++)
		{
			float y = pos[1];
			for (int j = 0; j < size[1]; j++)
			{
				float x = pos[0];
				for (int k = 0; k < size[2]; k++)
				{
					Entity entity = CreateEntity();
					glm::vec3 position = glm::vec3(x, y, z);
					entity.AddComponent<TransformComponent>(position);
					entity.AddComponent<RigidBodyComponent>(1.f, radius);
					entity.AddComponent<ColliderComponent>(radius);
					entity.AddComponent<ShapeComponent>(new Sphere(radius), sphereColors[(k + j + i) % 4]);
					x += 0.9f;
				}
				y += 0.9f;
			}
			z -= 0.9f;
		}
	}

}
