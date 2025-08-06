#include "Camera.h"

#include <GLFW\glfw3.h> // Maybe in future create QuackKeyCodes to not include this here only for key codes

#include "Input.h"


namespace Quack
{
	Camera::Camera()
	{
		auto [mouseX, mouseY] = Input::GetMousePos();
		lastX = mouseX;
		lastY = mouseY;
	}

	void Camera::Update(float dt)
	{
		float cameraSpeed = speed * dt;
		
		auto [mouseX, mouseY] = Input::GetMousePos();

		// Forward & Backward
		if (Input::IsKeyPressed(GLFW_KEY_W))
			position += front * cameraSpeed;
		if (Input::IsKeyPressed(GLFW_KEY_S))
			position -= front * cameraSpeed;	
		// Left & Right
		if (Input::IsKeyPressed(GLFW_KEY_A))
			position -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
		if (Input::IsKeyPressed(GLFW_KEY_D))
			position += glm::normalize(glm::cross(front, up)) * cameraSpeed;
		// Up & Down
		if (Input::IsKeyPressed(GLFW_KEY_E))
			position += up * cameraSpeed;
		if (Input::IsKeyPressed(GLFW_KEY_Q))
			position -= up * cameraSpeed;

		float offsetX = mouseX - lastX;
		float offsetY = lastY - mouseY;
		
		lastX = mouseX;
		lastY = mouseY;

		yaw += offsetX * cameraSpeed;
		pitch += offsetY * cameraSpeed;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		
		front = glm::normalize(direction);
	}

}