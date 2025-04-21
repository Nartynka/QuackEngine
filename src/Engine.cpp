#include "Engine.h"

#include <GLFW/glfw3.h>

namespace Quack
{
	Engine::Engine()
	{
		window = std::unique_ptr<Window>(Window::Create(1080, 720));
	}

	Engine::~Engine()
	{
		window->Shutdown();
	}

	void Engine::Run()
	{
		while (!glfwWindowShouldClose(window->GetWindow()))
		{
			glClearColor(0.5f, 0.7f, 0.8f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			window->Update();
		}
	}
}