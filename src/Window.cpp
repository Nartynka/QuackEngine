#include "Window.h"

#include <cassert>
#include <GLFW\glfw3.h>

namespace Quack
{
	void ResizeCallback(GLFWwindow* window, int width, int height);

	Window::Window(unsigned int width, unsigned int height) : width(width), height(height)
	{
		Init(width, height);
	}

	void Window::Init(unsigned int width, unsigned int height)
	{
		int result = glfwInit();

		assert(result && "Could not initialize GLFW!");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(width, height, "Quack Engine!", nullptr, nullptr);

		assert(window && "Could not create window!");

		glfwMakeContextCurrent(window);

		glfwSetFramebufferSizeCallback(window, ResizeCallback);

		// vsync
		glfwSwapInterval(1);
	}

	void ResizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	Window::~Window()
	{
		Shutdown();
	}

	Window* Window::Create(unsigned int width, unsigned int height)
	{
		return new Window(width, height);
	}

	void Window::Update()
	{
		glfwPollEvents();
		ProcessInput();

		glfwSwapBuffers(window);
	}

	GLFWwindow* Window::GetWindow()
	{
		return window;
	}

	void Window::ProcessInput()
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}