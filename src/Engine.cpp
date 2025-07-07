#include "Engine.h"

#include "Window.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Log.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Model.h"

namespace Quack
{
	Engine::Engine()
	{
		Log::Init();
		window = std::unique_ptr<Window>(Window::Create(1080, 720));
		renderer = std::unique_ptr<Renderer>(Renderer::Create());
		QUACK_LOG("Hello Engine!");
	}

	Engine::~Engine()
	{
		window->Shutdown();
	}

	void Engine::Run()
	{
		Shader shader("res/shaders/Basic.shader");
		shader.Bind();

		glm::vec3 pos(-30.0f, 0.0f, -100.0f);
		glm::vec3 pos2(3.0f, 0.0f, -10.0f);

		glm::mat4 model;
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
		
		shader.SetUniform4fv("view", glm::value_ptr(view));
		shader.SetUniform4fv("projection", glm::value_ptr(projection));

		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		Model* duck = new Model("./res/models/duck.obj");
		Model* cube = new Model("./res/models/cube.obj");

		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			glClear(GL_COLOR_BUFFER_BIT);

			shader.SetUniform4f("color", 0.5f, 0.0f, 0.5f);
			model = glm::translate(glm::mat4(1.0f), pos);
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(0.2f));
			shader.SetUniform4fv("model", glm::value_ptr(model));

			duck->Draw(shader);

			shader.SetUniform4f("color", 0.0f, 0.5f, 0.5f);
			model = glm::translate(glm::mat4(1.0f), pos2);
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.3f));
			shader.SetUniform4fv("model", glm::value_ptr(model));

			cube->Draw(shader);


			window->Update();
		}
	}
}