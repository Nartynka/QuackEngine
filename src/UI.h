#pragma once

struct GLFWwindow;

namespace Quack
{
	class UI
	{
	public:
		UI(GLFWwindow* window);
		~UI();
		
		void StartFrame();
		void EndFrame();

		void Shutdown();
	};
}