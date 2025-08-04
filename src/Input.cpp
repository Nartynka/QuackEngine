#include "Input.h"

#include <GLFW\glfw3.h>

namespace Quack
{
	GLFWwindow* Input::window = nullptr;

	void Input::SetWindow(GLFWwindow* w)
	{
		window = w;
	}

	bool Input::IsKeyPressed(int keyCode)
	{
		int state = glfwGetKey(window, keyCode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		int state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	std::pair<float, float> Input::GetMousePos()
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		return { (float)x, (float)y };
	}
}
