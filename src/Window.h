#pragma once

#include <memory>

struct GLFWwindow;

namespace Quack
{
	class UI;

	class Window
	{
	public:
		static Window* Create(unsigned int width, unsigned int height);
		~Window();
		
		void Update();

		GLFWwindow* GetWindow();

		void Shutdown();
	private:
		Window(unsigned int width, unsigned int height);

		void Init(unsigned int width, unsigned int height);
		static void ProcessInput(GLFWwindow* window, int key, int scancode, int action, int mods);

		unsigned int width;
		unsigned int height;
		GLFWwindow* window;
		// ui will be moved to renderer later
		std::unique_ptr<UI> ui;
	};
}