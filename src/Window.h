#pragma once

struct GLFWwindow;

namespace Quack
{
	class Window
	{
	public:
		static Window* Create(unsigned int width, unsigned int height);
		
		void Update();

		GLFWwindow* GetWindow();

		void Shutdown();
		~Window();
	private:
		Window(unsigned int width, unsigned int height);

		void Init(unsigned int width, unsigned int height);
		void ProcessInput();

		unsigned int width;
		unsigned int height;
		GLFWwindow* window;
	};
}