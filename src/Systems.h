#pragma once

#include <memory>
#include <glm.hpp>

namespace Quack
{
	class Scene;
	class Shader;
	struct ConstraintComponent;

	void RenderShapes(const std::shared_ptr<Scene> scene, Shader& shader);

	void RenderModels(const std::shared_ptr<Scene> scene, Shader& shader);

	void RenderCollisionShapes(const std::shared_ptr<Scene> scene);


	//void Move(const std::shared_ptr<Scene> scene, float dt);
	
	//void CheckCollision(const std::shared_ptr<Scene> scene, float dt, glm::vec3 floorPos, glm::vec3 floorHalfSize); // and resolve collision

	void ApplyForces(const std::shared_ptr<Scene> scene);

	void Update(const std::shared_ptr<Scene> scene, float dt);

	void UpdateTransform(const std::shared_ptr<Scene> scene);

	void SolveConstraints(const std::shared_ptr<Scene> scene);

	void SolveCollision(const std::shared_ptr<Scene> scene); // collision between spheres
}
