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

namespace Quack
{
	Engine::Engine()
	{
		window = std::unique_ptr<Window>(Window::Create(1080, 720));
		renderer = std::unique_ptr<Renderer>(Renderer::Create());
		Log::Init();
		QUACK_LOG("Hello Engine!");
	}

	Engine::~Engine()
	{
		window->Shutdown();
	}

	static float vertices[] = {
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f,
	};

	static int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	void Engine::Run()
	{
		VertexArray va;
		VertexBuffer vb(vertices, 6 * 2 * sizeof(float));

		VertexBufferLayout layout;
		layout.AddElement(2);

		va.AddBuffer(vb, layout);

		IndexBuffer ib(indices, 6);

		Shader shader("res/shaders/Basic.shader");
		shader.Bind();

		glm::vec3 pos(-1.0f, 0.0f, -5.0f);
		glm::vec3 pos2(1.0f, 0.0f, -5.0f);

		glm::mat4 model;
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		
		shader.SetUniform4fv("view", glm::value_ptr(view));
		shader.SetUniform4fv("projection", glm::value_ptr(projection));
		
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			glClear(GL_COLOR_BUFFER_BIT);

			shader.SetUniform4f("color", 0.0f, 0.5f, 0.5f);
			model = glm::translate(glm::mat4(1.0f), pos);
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
			shader.SetUniform4fv("model", glm::value_ptr(model));

			renderer->Draw(va, ib, shader);

			shader.SetUniform4f("color", 0.5f, 0.0f, 0.5f);
			model = glm::translate(glm::mat4(1.0f), pos2);
			model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
			shader.SetUniform4fv("model", glm::value_ptr(model));

			renderer->Draw(va, ib, shader);

			window->Update();
		}
	}
}