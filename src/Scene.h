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

		int GetEntityCount();
		void ClearEntities();

		void SpawnFloorScene();
		void SpawnTestScene();
		void SpawnTiltedFloorsScene();

		void SpawnCribbingTower();

		void SpawnStackOfCubes(int* size, float* pos);
		void SpawnStackOfSpheres(int* size, float* pos);

	private:
		entt::registry registry;

	};
}
