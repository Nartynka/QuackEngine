#include "Window.h"

#include "UI.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <cassert>
#include "Log.h"

namespace Quack
{
	void ResizeCallback(GLFWwindow* window, int width, int height);
	void ErrorCallback(int error, const char* description);

	Window::Window(unsigned int width, unsigned int height) 
		: width(width), height(height)
	{
		Init(width, height);
		ui = std::unique_ptr<UI>(UI::Create(window));
	}
	
	Window* Window::Create(unsigned int width, unsigned int height)
	{
		return new Window(width, height);
	}

	Window::~Window()
	{
		//ui->Shutdown();
		Shutdown();
	}

	void Window::Init(unsigned int width, unsigned int height)
	{	
		int glfwResult = glfwInit();
		assert(glfwResult && "Could not initialize GLFW!");

		window = glfwCreateWindow(width, height, "Quack Engine!", NULL, NULL);
		assert(window && "Could not create window!");

		glfwMakeContextCurrent(window);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwSetErrorCallback(ErrorCallback);
		glfwSetFramebufferSizeCallback(window, ResizeCallback);
		glfwSetKeyCallback(window, ProcessInput);
		
		// vsync
		glfwSwapInterval(1);

		// init glew
		int glewResult = glewInit();
		assert(!glewResult && "Could not initialize GLEW!");
	}

	void Window::Update()
	{	
		ui->StartFrame();
		ui->EndFrame();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// @TODO: Should this be here? Maybe it should be moved to separate input class
	void Window::ProcessInput(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		if (key == GLFW_KEY_F && action == GLFW_PRESS)
			QUACK_GOOD("Paying respects")
	}

	void ResizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void ErrorCallback(int error, const char* description)
	{
		QUACK_ERROR("glfw error: {}", description);
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}
