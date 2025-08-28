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

	Entity Scene::CreateEntity()
	{
		return Entity{ registry.create(), this };
	}

	entt::registry& Scene::GetRegistry()
	{
		return registry;
	}

	int Scene::GetEntitiesCount()
	{
		return registry.storage<entt::entity>().size();
	}

	//void Scene::DestroyEntity(const Entity& entity)
	//{

	//}

}