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

		void Init();

		Entity CreateEntity();

		entt::registry& GetRegistry();

		int GetEntityCount();
		void ClearEntities();

		void SpawnStackOfCubes(int* size, float* pos);
		void SpawnStackOfSpheres(int* size, float* pos);

		void SpawnFloor();
	private:
		entt::registry registry;

	};
}
