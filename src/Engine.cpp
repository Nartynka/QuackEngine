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

namespace Quack
{
	Engine::Engine()
	{
		Log::Init();
		window = std::make_unique<Window>(1080, 720);
		// bind or lambda that is the question :b
		window->SetCallback(std::bind(&Engine::OnEvent, this, std::placeholders::_1));
		renderer = std::make_unique<Renderer>();
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
		dispatcher.Dispatch<MouseLeftButtonPressedEvent>(std::bind(&Engine::OnMouseButton, this, std::placeholders::_1));
		dispatcher.Dispatch<MouseRightButtonPressedEvent>([](const MouseRightButtonPressedEvent& e) {QUACK_GOOD("Right mouse button pressed!!"); });
	}

	struct TransformComponent
	{
		glm::mat4 transform;

		TransformComponent(glm::mat4 transform)
			: transform(transform) {}

		operator glm::mat4()& { return transform; }
	};

	struct PhysicsComponent
	{
		glm::vec3 velocity;
		glm::vec3 acceleration = glm::vec3(0.0f);

		float mass;

		PhysicsComponent(glm::vec3 velocity, glm::vec3 acceleration, float mass)
			: velocity(velocity), acceleration(acceleration), mass(mass) {}
	};

	void Engine::OnMouseButton(const MouseLeftButtonPressedEvent& e)
	{
		QUACK_GOOD("Left mouse button pressed!!");

		static glm::vec3 pos(-13.0f, 0.0f, -20.0f);
		pos.x += 3.f;

		entt::entity entity	= registry.create();
		registry.emplace<TransformComponent>(entity, glm::translate(glm::mat4(1.0f), pos));
	}

	void Engine::Run()
	{
		Shader shader("res/shaders/Basic.shader");
		shader.Bind();

		glm::mat4 model;
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
		
		shader.SetUniform4fv("view", glm::value_ptr(view));
		shader.SetUniform4fv("projection", glm::value_ptr(projection));

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		Model* duck = new Model("res/models/duck.fbx");

		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			auto view = registry.view<TransformComponent>();
			for (auto entity : view)
			{
				TransformComponent& transform = registry.get<TransformComponent>(entity);
				model = transform;
				model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 0.5f, 0.0f));
				shader.SetUniform4fv("model", glm::value_ptr(model));

				duck->Draw(shader);
			}
			window->Update();
		}
	}
}
