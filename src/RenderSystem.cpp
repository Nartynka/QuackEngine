#include "Systems.h"

#include "Scene.h"
#include "Shader.h"
#include "Renderer.h"

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>


namespace Quack
{
	void RenderShapes(const std::shared_ptr<Scene> scene, Shader& shader)
	{
		auto& registry = scene->GetRegistry();
		auto shapeView = registry.view<ShapeComponent, TransformComponent>();
		
		for (auto entity : shapeView)
		{
			auto& [shape, transform] = shapeView.get(entity);
			glm::mat4 model = transform.transform;
			//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 0.5f, 0.0f));

			shader.SetUniform4fv("model", glm::value_ptr(model));
			shader.SetUniform4f("inColor", 0.5f, 0.0f, 0.5f);

			Renderer::Draw(*shape.shape->vao, *shape.shape->ibo, shader);
		}
	}

	void RenderModels(const std::shared_ptr<Scene> scene, Shader& shader)
	{
		auto& registry = scene->GetRegistry();
		auto modelView = registry.view<ModelComponent, TransformComponent>();
		
		for (auto entity : modelView)
		{
			auto& [modelComp, transform] = modelView.get(entity);

			glm::mat4 model = transform.transform;
			//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 0.5f, 0.0f));
			model = glm::scale(model, glm::vec3(0.5f));

			shader.SetUniform4fv("model", glm::value_ptr(model));
			shader.SetUniform4f("inColor", 0.0f, 0.0f, 0.0f, 0.0f);

			modelComp.model->Draw(shader);

		}
	}

	void RenderCollisionShapes(const std::shared_ptr<Scene> scene, Shader& shader)
	{
		auto& registry = scene->GetRegistry();
		auto collisionView = registry.view<CollisionComponent, TransformComponent>();

		for (auto entity : collisionView)
		{
			auto& [collision, transform] = collisionView.get(entity);

			shader.SetUniform4fv("model", glm::value_ptr(transform.transform));
			shader.SetUniform4f("inColor", 0.0f, 1.f, 0.5f);
			Renderer::DrawOutline(*collision.shape->vao, shader);
		}
	}

}
