#pragma once

#include <entt.hpp>

#include "Entity.h"

namespace Quack
{
	// class responsible for managing active objects (entities)
	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity();

		entt::registry& GetRegistry();
		//void DestroyEntity(const Entity& entity); // delete, remove?
		//void Draw();
	private:
		entt::registry registry;
	};
}
