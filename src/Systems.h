#pragma once

#include <memory>
#include <glm.hpp>

namespace Quack
{
	class Scene;
	class Shader;

	void RenderShapes(const std::shared_ptr<Scene> scene, Shader& shader);

	void RenderModels(const std::shared_ptr<Scene> scene, Shader& shader);

	void RenderCollisionShapes(const std::shared_ptr<Scene> scene);

	// Physics
	void ApplyForces(const std::shared_ptr<Scene> scene);

	void Update(const std::shared_ptr<Scene> scene, float dt);

	void UpdateTransform(const std::shared_ptr<Scene> scene);

	void CheckCollision(const std::shared_ptr<Scene> scene);

	void SolveCollision(const std::shared_ptr<Scene> scene);
}
