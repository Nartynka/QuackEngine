#pragma once

struct GLFWwindow;

namespace Quack
{
	class UI
	{
	public:
		static UI* Create(GLFWwindow* window);
		~UI();
		
		void StartFrame();
		void EndFrame();

		void Shutdown();

	private:
		UI(GLFWwindow* window);

	};
}