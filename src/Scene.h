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

		const char* GetCurrentSceneName();

		float dt;

		bool bWarmStart = true;
		bool bPositionalCorrection = false;
		bool isWireframeMode = false;
		bool isPaused = false;

	private:
		entt::registry registry;

		const char* sceneName[3] = { "Default", "Test", "Tilted Floors"};
		int currentScene = 0;
	};
}
