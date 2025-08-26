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
		
		shader.Bind();
		for (auto entity : shapeView)
		{
			auto& [shape, transform] = shapeView.get(entity);
			glm::mat4 model = transform.transform;
			//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 0.5f, 0.0f));
			shader.SetUniform4fv("model", glm::value_ptr(model));

			if (shape.shape->ibo)
			{
				shader.SetUniform4f("inColor", 0.5f, 0.0f, 0.5f);
				Renderer::Draw(*shape.shape->vao, *shape.shape->ibo, shader);
			}
			else
			{
				shader.SetUniform4f("inColor", 0.0f, 0.5f, 0.5f);
				Renderer::DrawNotIndexed(*shape.shape->vao, shape.shape->GetVerticesCount(), shader);
			}
		}
	}

	void RenderModels(const std::shared_ptr<Scene> scene, Shader& shader)
	{
		auto& registry = scene->GetRegistry();
		auto modelView = registry.view<ModelComponent, TransformComponent>();
		
		shader.Bind();
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

	void RenderCollisionShapes(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto collisionView = registry.view<CollisionComponent, TransformComponent>();

		for (auto entity : collisionView)
		{
			auto& [collision, transform] = collisionView.get(entity);

			Renderer::DrawOutline(*collision.shape->vao, *collision.shape->ibo, transform.transform, glm::vec3(0.f, 0.5f, 1.f));
		}
	}

}
