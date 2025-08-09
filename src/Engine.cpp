#include "Engine.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <functional>

#include "Window.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Log.h"
#include "Model.h"
#include "KeyEvent.h"
#include "MouseEvent.h"
#include "Input.h"
#include "Camera.h"

#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "Shapes.h"
#include "ModelLibrary.h"

namespace Quack
{
	Engine::Engine()
	{
		Log::Init();

		window = std::make_unique<Window>(1080, 720);
		window->SetCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1)); // bind or lambda that is the question :b
		
		Input::SetWindow(window->GetWindow());
		
		renderer = std::make_unique<Renderer>();

		scene = std::make_unique<Scene>();

		camera = std::make_unique<Camera>();

		ModelLibrary::Init();

		QUACK_LOG("Hello Engine!");
	}

	Engine::~Engine()
	{
		window->Shutdown();
	}

	void Engine::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<KeyPressedEvent>([](const KeyPressedEvent& e) { QUACK_GOOD("Key Pressed!!! key code: {}", e.GetKeyCode()); });
		dispatcher.Dispatch<MouseLeftButtonPressedEvent>(std::bind(&Engine::OnLeftMouseButton, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseRightButtonPressedEvent>(std::bind(&Engine::OnRightMouseButton, this, std::placeholders::_1));
		//dispatcher.Dispatch<MouseMovedEvent>([](const MouseMovedEvent& e) {QUACK_GOOD("Mouse Moved!!!"); });
		dispatcher.Dispatch<MouseScrolledEvent>([&](const MouseScrolledEvent& e) {QUACK_GOOD("Mouse Scrolled!!!"); camera->speed += e.offsetY; camera->speed = camera->speed < 1.f ? 1.f : camera->speed; });
	}

	void Engine::OnLeftMouseButton(const MouseLeftButtonPressedEvent& e)
	{
		QUACK_LOG("Left mouse button pressed!!");

		Entity entity = scene->CreateEntity();

		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 5.f, -5.f)));
		entity.AddComponent<PhysicsComponent>(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -9.8f, 0.f));
		entity.AddComponent<CollisionComponent>(glm::vec3(0.3f, 0.41f, 0.5f));
		entity.AddComponent<ModelComponent>(ModelLibrary::duck.get());
	}

	void Engine::OnRightMouseButton(const MouseRightButtonPressedEvent& e)
	{
		QUACK_LOG("Right mouse button pressed!!");

		Entity entity = scene->CreateEntity();

		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 5.f, -5.f)));
		entity.AddComponent<PhysicsComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -9.8f, 0.f));
		entity.AddComponent<CollisionComponent>(glm::vec3(0.25f));
		entity.AddComponent<ShapeComponent>(new Cube(glm::vec3(0.25f)));
	}

	void Engine::Run()
	{
		const float DESIRED_DT = 1 / 60.f; // 60 FPS

		Shader shader("res/shaders/Basic.shader");
		shader.Bind();

		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

		shader.SetUniform4fv("projection", glm::value_ptr(projection));

		
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		auto& registry = scene->GetRegistry(); // @TODO: remove this and make systems for ECS
		float lastTime = 0.f;

		Rectangle floor(glm::vec3(3.5f, 0.1f, 4.5f));
		Rectangle floor2(glm::vec3(3.5f, 0.2f, 4.5f));

		glm::vec3 floorPos = glm::vec3(0.f, -0.5f, -5.f);
		glm::vec3 floorCollisionHalfSize = glm::vec3(3.5f, 0.1f, 4.5f);

		glm::vec3 minFloor;
		minFloor.x = floorPos.x - floorCollisionHalfSize.x;
		minFloor.y = floorPos.y - floorCollisionHalfSize.y;
		minFloor.z = floorPos.z - floorCollisionHalfSize.z;

		glm::vec3 maxFloor;
		maxFloor.x = floorPos.x + floorCollisionHalfSize.x;
		maxFloor.y = floorPos.y + floorCollisionHalfSize.y;
		maxFloor.z = floorPos.z + floorCollisionHalfSize.z;


		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			float currentTime = (float)glfwGetTime(); // time since glfw initialization in seconds
			float dt = currentTime - lastTime;
			lastTime = currentTime;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			camera->Update(dt);

			// Maybe view & projection should be in camera class?
			// first vector moves the scene???
			// camera move is the inverse of what we want to do? inverse the direction of where we want to go???
			// the second vector is the point that we look at 
			view = glm::lookAt(camera->position, camera->position+camera->front, camera->up);
			shader.SetUniform4fv("view", glm::value_ptr(view));	

			auto physicsView = registry.view<PhysicsComponent, TransformComponent>();
			// "physics system?"
			// entity is just a uint32_t so no need for a const reference
			for (auto entity : physicsView)
			{
				auto& [physics, transform] = physicsView.get(entity);
				physics.velocity += physics.acceleration * dt;
				transform.transform = glm::translate(transform.transform, physics.velocity * dt);
			}
			
			// collision and physics should be one component / system
			// "collision system?"
			// For now only checks collision with floor
			auto collisionView = registry.view<CollisionComponent, TransformComponent, PhysicsComponent>();
			for (auto entity : collisionView)
			{
				auto& [collision, transform, physics] = collisionView.get(entity);

				glm::vec3 minEntity;
				minEntity.x = transform.transform[3][0] - collision.halfSize.x;
				minEntity.y = transform.transform[3][1] - collision.halfSize.y;
				minEntity.z = transform.transform[3][2] - collision.halfSize.z;

				glm::vec3 maxEntity;
				maxEntity.x = transform.transform[3][0] + collision.halfSize.x;
				maxEntity.y = transform.transform[3][1] + collision.halfSize.y;
				maxEntity.z = transform.transform[3][2] + collision.halfSize.z;

				if ((minEntity.x <= maxFloor.x && maxEntity.x >= minFloor.x) &&
					(minEntity.y <= maxFloor.y && maxEntity.y >= minFloor.y) &&
					(minEntity.z <= maxFloor.z && maxEntity.z >= minFloor.z))
				{
					QUACK_LOG("Collllision!!!!!");

					transform.transform = glm::translate(transform.transform, -(physics.velocity * dt));

					physics.velocity = glm::vec3(0.f);
					//physics.acceleration = glm::vec3(0.f); 'Oh gravity, thou art a heartless bitch' - Jim Parsons
				}

				// Draw collision shapes
				shader.SetUniform4fv("model", glm::value_ptr(transform.transform));
				shader.SetUniform4f("inColor", 0.0f, 1.f, 0.5f);
				renderer->DrawOutline(*collision.shape->vao, shader);
			}


			// "render system?"
			auto modelView = registry.view<ModelComponent, TransformComponent>();
			for (auto entity : modelView)
			{
				auto& [modelComp, transform] = modelView.get(entity);

				model = transform.transform;
				//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 0.5f, 0.0f));
				model = glm::scale(model, glm::vec3(0.5f));

				shader.SetUniform4fv("model", glm::value_ptr(model));
				shader.SetUniform4f("inColor", 0.0f, 0.0f, 0.0f, 0.f);
				
				modelComp.model->Draw(shader);
			}

			auto shapeView = registry.view<ShapeComponent, TransformComponent>();
			for (auto entity : shapeView)
			{
				auto& [shape, transform] = shapeView.get(entity);

				model = transform.transform;
				//model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 0.5f, 0.0f));

				shader.SetUniform4fv("model", glm::value_ptr(model));
				shader.SetUniform4f("inColor", 0.5f, 0.0f, 0.5f);

				renderer->Draw(*shape.shape->vao, *shape.shape->ibo, shader);
				shader.SetUniform4f("inColor", 0.0f, 1.f, 0.5f);
				//renderer->DrawOutline(*shape.shape->vao, shader);
			}

			model = glm::translate(glm::mat4(1.0f), floorPos);
			shader.SetUniform4fv("model", glm::value_ptr(model));
			shader.SetUniform4f("inColor", 0.0f, 0.5f, 0.5f);
			renderer->Draw(*floor.vao, *floor.ibo, shader);

			model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -0.5f, 5.f));
			shader.SetUniform4fv("model", glm::value_ptr(model));
			shader.SetUniform4f("inColor", 0.5f, 0.5f, 0.0f);
			renderer->Draw(*floor.vao, *floor.ibo, shader);

			window->Update();
		}
	}
}
