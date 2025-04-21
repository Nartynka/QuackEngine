#include "Window.h"

#include "UI.h"

#include <cassert>
#include <GLFW\glfw3.h>

namespace Quack
{
	void ResizeCallback(GLFWwindow* window, int width, int height);
	void ErrorCallback(int error, const char* description);

	Window::Window(unsigned int width, unsigned int height) : width(width), height(height)
	{
		Init(width, height);
		ui = std::unique_ptr<UI>(UI::Create(window));
	}

	Window::~Window()
	{
		//ui->Shutdown();
		Shutdown();
	}

	void Window::Init(unsigned int width, unsigned int height)
	{
		glfwSetErrorCallback(ErrorCallback);
		
		int result = glfwInit();

		assert(result && "Could not initialize GLFW!");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(width, height, "Quack Engine!", nullptr, nullptr);

		assert(window && "Could not create window!");

		glfwMakeContextCurrent(window);

		glfwSetFramebufferSizeCallback(window, ResizeCallback);
		glfwSetKeyCallback(window, ProcessInput);
		
		glEnable(GL_DEPTH_TEST);
		
		// vsync
		glfwSwapInterval(1);

	}

	Window* Window::Create(unsigned int width, unsigned int height)
	{
		return new Window(width, height);
	}

	void Window::Update()
	{
		glfwPollEvents();
		ui->StartFrame();
		ui->EndFrame();
		glfwSwapBuffers(window);
	}

	// @TODO: Should this be here? Maybe it should be moved to separate input class
	void Window::ProcessInput(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		if (key == GLFW_KEY_F && action == GLFW_PRESS)
			printf("Paying respects\n");
	}

	GLFWwindow* Window::GetWindow()
	{
		return window;
	}

	void ResizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void ErrorCallback(int error, const char* description)
	{
		printf("Error: %s\n", description);
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}