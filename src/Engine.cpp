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

#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "Shapes.h"

namespace Quack
{
	Engine::Engine()
	{
		Log::Init();

		window = std::make_unique<Window>(1080, 720);
		window->SetCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1)); // bind or lambda that is the question :b

		renderer = std::make_unique<Renderer>();

		scene = new Scene();

		QUACK_LOG("Hello Engine!");
	}

	Engine::~Engine()
	{
		window->Shutdown();
	}

	void Engine::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);

		dispatcher.Dispatch<KeyPressedEvent>([](const KeyPressedEvent& e) {QUACK_GOOD("Key Pressed!!! key code: {}", e.keyCode); });
		dispatcher.Dispatch<MouseLeftButtonPressedEvent>(std::bind(&Engine::OnLeftMouseButton, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseRightButtonPressedEvent>(std::bind(&Engine::OnRightMouseButton, this, std::placeholders::_1));
	}
	
	struct Camera
	{
		float speed = 5.f;

		glm::vec3 position = glm::vec3(0.f, 0.f, 3.f);
		glm::vec3 front = glm::vec3(0.f, 0.f, -1.f);
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);

	};

	static Camera camera;

	void Engine::ProcessInput(float dt)
	{
		float cameraSpeed = camera.speed * dt;
		if (glfwGetKey(window->GetWindow(), GLFW_KEY_W) == GLFW_PRESS)
			camera.position += camera.front * cameraSpeed;
		if (glfwGetKey(window->GetWindow(), GLFW_KEY_S) == GLFW_PRESS)
			camera.position -= camera.front * cameraSpeed;
		if (glfwGetKey(window->GetWindow(), GLFW_KEY_A) == GLFW_PRESS)
			camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) * cameraSpeed;
		if (glfwGetKey(window->GetWindow(), GLFW_KEY_D) == GLFW_PRESS)
			camera.position += glm::normalize(glm::cross(camera.front, camera.up)) * cameraSpeed;
	}


	void Engine::OnLeftMouseButton(const MouseLeftButtonPressedEvent& e)
	{
		QUACK_GOOD("Left mouse button pressed!!");
		double x, y;
		glfwGetCursorPos(window->GetWindow(), &x, &y);
		Entity entity = scene->CreateEntity();

		// convert mouse position to NDC
		x = (x / (1080 / 2)) - 1;
		y = -(y / (720 / 2)) + 1;

		static Model* duck = new Model("res/models/duck.fbx");

		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(x * 5, y * 5, -10.f)));
		entity.AddComponent<PhysicsComponent>(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -9.8f, 0.f));
		entity.AddComponent<ModelComponent>(duck);
	}

	void Engine::OnRightMouseButton(const MouseRightButtonPressedEvent& e)
	{
		QUACK_GOOD("Right mouse button pressed!!");
		double x, y;
		glfwGetCursorPos(window->GetWindow(), &x, &y);
		Entity entity = scene->CreateEntity();

		// convert mouse position to NDC
		x = (x / (1080 / 2)) - 1;
		y = -(y / (720 / 2)) + 1;

		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(x * 5, y * 5, -10.f)));
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

		Entity entity = scene->CreateEntity();
		entity.AddComponent<TransformComponent>(glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.f, 0.f)));
		entity.AddComponent<ShapeComponent>(new Cube());


		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			float currentTime = (float)glfwGetTime(); // time since glfw initialization in seconds
			float dt = currentTime - lastTime;
			lastTime = currentTime;

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ProcessInput(dt);
			// first vector moves the scene??? 
			// camera move is the inverse of what we want to do? inverse the direction of where we want to go???
			// the second vector is the point that we look at 
			view = glm::lookAt(camera.position, camera.position+camera.front, camera.up);
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

			window->Update();
		}
	}
}
