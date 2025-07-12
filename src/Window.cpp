#include "Window.h"

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include "UI.h"
#include "Log.h"
#include "Assert.h"
#include "KeyEvent.h"
#include "MouseEvent.h"

namespace Quack
{
	Window::Window(unsigned int width, unsigned int height) 
		: width(width), height(height)
	{
		Init(width, height);
		ui = std::make_unique<UI>(window);
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
				if (button == GLFW_MOUSE_BUTTON_LEFT)
				{
					MouseLeftButtonPressedEvent event;
					data->callback(event);
				}
				else if (button == GLFW_MOUSE_BUTTON_RIGHT)
				{
					MouseRightButtonPressedEvent event;
					data->callback(event);
				}
			}
		});
		
		// vsync
		glfwSwapInterval(1);

		// init glew
		int glewResult = glewInit();
		QUACK_ASSERT(!glewResult, "Could not initialize GLEW!");
	}

	void Window::Update()
	{	
		ui->StartFrame();
		ui->EndFrame();
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}
