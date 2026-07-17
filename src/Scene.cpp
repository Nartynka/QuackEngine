#include "Scene.h"

#include <glm.hpp>

namespace Quack
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
	}

	void Scene::Init()
	{
		SpawnFloor();
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

	void Scene::SpawnFloor()
	{
		glm::vec3 floorHalfSize = glm::vec3(3.5f, 0.1f, 4.5f);
		glm::vec4 floorColor = glm::vec4(0.0f, 0.5f, 0.5f, 1.0f);

		Entity floor = CreateEntity();
		glm::vec3 floorPos = glm::vec3(-2.f, 0.f, -5.f);
		floor.AddComponent<TransformComponent>(floorPos);
		floor.AddComponent<ShapeComponent>(new NormalCube(floorHalfSize), floorColor);
		floor.AddComponent<ColliderComponent>(floorHalfSize);
	}

}
