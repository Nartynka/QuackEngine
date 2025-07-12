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

	static std::vector<Model*> models;

	void Engine::OnMouseButton(const MouseLeftButtonPressedEvent& e)
	{
		QUACK_GOOD("Left mouse button pressed!!");
		models.push_back(models[0]);
	}

	void Engine::Run()
	{
		Shader shader("res/shaders/Basic.shader");
		shader.Bind();

		//glm::vec3 pos(0.0f, 0.0f, -20.0f);
		//glm::vec3 pos2(3.0f, 0.0f, -20.0f);

		glm::mat4 model;
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
		
		shader.SetUniform4fv("view", glm::value_ptr(view));
		shader.SetUniform4fv("projection", glm::value_ptr(projection));

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		//Model* duck = new Model("./res/models/duck.fbx");
		//Model* cube = new Model("./res/models/cube.fbx");

		Model* duck = new Model("res/models/duck.fbx");
		models.push_back(duck);


		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glm::vec3 pos(-13.0f, 0.0f, -20.0f);

			for (auto* m : models)
			{
				model = glm::translate(glm::mat4(1.0f), pos);
				model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 0.5f, 0.0f));
				shader.SetUniform4fv("model", glm::value_ptr(model));
				m->Draw(shader);
				pos.x += 3;
			}
			window->Update();
		}
	}
}
