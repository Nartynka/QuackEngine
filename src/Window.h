#pragma once

#include <memory>

struct GLFWwindow;

namespace Quack
{
	class UI;

	class Window
	{
	public:
		Window(unsigned int width, unsigned int height);
		~Window();
		
		void Update();

		inline GLFWwindow* GetWindow() const { return window; }

		void Shutdown();
		void Init(unsigned int width, unsigned int height);
	private:
		static void ProcessInput(GLFWwindow* window, int key, int scancode, int action, int mods);

		GLFWwindow* window;

		unsigned int width;
		unsigned int height;

		std::unique_ptr<UI> ui;
	};
}