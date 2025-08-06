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
		dispatcher.Dispatch<MouseScrolledEvent>([](const MouseScrolledEvent& e) {QUACK_GOOD("Mouse Scrolled!!!"); });
	}

	void Engine::OnLeftMouseButton(const MouseLeftButtonPressedEvent& e)
	{
		QUACK_LOG("Left mouse button pressed!!");

		Entity entity = scene->CreateEntity();

		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 5.f, -10.f)));
		entity.AddComponent<PhysicsComponent>(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -9.8f, 0.f));
		entity.AddComponent<ModelComponent>(ModelLibrary::duck.get());
	}

	void Engine::OnRightMouseButton(const MouseRightButtonPressedEvent& e)
	{
		QUACK_LOG("Right mouse button pressed!!");

		Entity entity = scene->CreateEntity();

		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 5.f, -10.f)));
		entity.AddComponent<PhysicsComponent>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, -9.8f, 0.f));
		entity.AddComponent<ShapeComponent>(new Cube());
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

		Rectangle floor;
		Rectangle floor2;

		Entity entity = scene->CreateEntity();
		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, 0.f)));
		entity.AddComponent<ShapeComponent>(new Cube());

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

			// "render system?"
			auto modelView = registry.view<ModelComponent, TransformComponent>();
			for (auto entity : modelView)
			{
				auto& [modelComp, transform] = modelView.get(entity);

				model = transform.transform;
				model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 0.5f, 0.0f));
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
				model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 0.5f, 0.0f));
				model = glm::scale(model, glm::vec3(0.5f));

				shader.SetUniform4fv("model", glm::value_ptr(model));
				shader.SetUniform4f("inColor", 0.5f, 0.0f, 0.5f);

				renderer->Draw(*shape.shape->vao, *shape.shape->ibo, shader);
			}

			model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -0.5f, -5.f));
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
