#include "Engine.h"

#include "Window.h"
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>


namespace Quack
{
	Engine::Engine()
	{
		window = std::unique_ptr<Window>(Window::Create(1080, 720));
		renderer = std::unique_ptr<Renderer>(Renderer::Create());
	}

	Engine::~Engine()
	{
		window->Shutdown();
	}

	void Engine::Run()
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			glClear(GL_COLOR_BUFFER_BIT);
			renderer->Draw();
			window->Update();
		}
	}
}