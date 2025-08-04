#pragma once
#include <utility>

struct GLFWwindow;

namespace Quack
{
	// I don't like this but idk what would be better or do I even need/want to abstract it
	class Input
	{
	public:
		static void SetWindow(GLFWwindow* w);

		static bool IsKeyPressed(int keyCode);
		static bool IsMouseButtonPressed(int button);
		static std::pair<float, float> GetMousePos();
	private:
		static GLFWwindow* window;
	};
}