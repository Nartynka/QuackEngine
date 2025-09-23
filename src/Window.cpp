#include "Window.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "Log.h"
#include "Assert.h"
#include "KeyEvent.h"
#include "MouseEvent.h"

namespace Quack
{
	Window::Window(unsigned int width, unsigned int height)
	{
		Init(width, height);
	}

	Window::~Window()
	{
		//ui->Shutdown();
		Shutdown();
	}

	static void ErrorCallback(int error, const char* description)
	{
		QUACK_ERROR("glfw error ({}): {}", error, description);
	}

	void Window::Init(unsigned int width, unsigned int height)
	{	
		int glfwResult = glfwInit();
		QUACK_ASSERT(glfwResult, "Could not initialize GLFW!");

		window = glfwCreateWindow(width, height, "Quack Engine!", NULL, NULL);
		QUACK_ASSERT(window, "Could not create window!");

		glfwMakeContextCurrent(window);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwSetErrorCallback(ErrorCallback);

		glfwSetWindowUserPointer(window, &data);

		glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
		{
			glViewport(0, 0, width, height);
		});

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
			
			if(action == GLFW_PRESS)
			{
				KeyPressedEvent event(key);
				data->callback(event);
			}
		});

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);
			
			if (action == GLFW_PRESS) 
			{
				MouseButtonPressedEvent event(button);
				data->callback(event);
			}
		});

		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) 
		{
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event(xpos, ypos);
			data->callback(event);
		});

		glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event(xoffset, yoffset);
			data->callback(event);
		});

		// vsync
		glfwSwapInterval(true);

		// init glew
		int glewResult = glewInit();
		QUACK_ASSERT(!glewResult, "Could not initialize GLEW!");
	}

	std::pair<int, int> Window::GetWindowSize() const
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		return { width, height };
	}

	void Window::Update()
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}
