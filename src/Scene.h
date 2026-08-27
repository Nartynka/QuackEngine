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
		void SpawnCribbingTowerScene();
		void SpawnPyramidScene(int size = 3, glm::vec3 center = { 0.f, 0.5f, -5.f });

		void SpawnFloor();
		void SpawnStackOfCubes(int* size, float* pos);
		void SpawnStackOfSpheres(int* size, float* pos);

		const char* GetCurrentSceneName();

		float dt;

		bool bWarmStart = true;
		bool bPositionalCorrection = true;
		bool isWireframeMode = false;
		bool isPaused = false;
		bool shouldDoOneStep = false;

	private:
		entt::registry registry;

		const char* sceneName[5] = { "Default", "Test", "Tilted Floors", "Cribbing Tower", "Pyramid"};
		int currentScene = 0;
	};
}
