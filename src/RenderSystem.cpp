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
			shader.SetUniform4fm("model", glm::value_ptr(model));

			shader.SetUniform4fv("inColor", glm::value_ptr(shape.color));
			if (shape.shape->ibo)
			{
				Renderer::Draw(*shape.shape->vao, *shape.shape->ibo, shader);
			}
			else
			{
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

			shader.SetUniform4fm("model", glm::value_ptr(model));
			shader.SetUniform4fv("inColor", glm::value_ptr(modelComp.color));

			modelComp.model->Draw(shader);

		}
	}

	void RenderCollisionShapes(const std::shared_ptr<Scene> scene)
	{
		auto& registry = scene->GetRegistry();
		auto view = registry.view<ColliderComponent, TransformComponent>();

		for (auto entity : view)
		{
			auto& [collision, transform] = view.get(entity);

			// @TODO: think about better way to do it
			// remove scale from transformation matrix
			glm::mat4 trans = transform.transform;
			trans[0] = normalize(transform.transform[0]);
			trans[1] = normalize(transform.transform[1]);
			trans[2] = normalize(transform.transform[2]);

			Renderer::DrawOutline(*collision.shape->vao, *collision.shape->ibo, trans, glm::vec3(0.f, 0.5f, 1.f));
		}
	}

}
