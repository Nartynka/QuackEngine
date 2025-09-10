#include "Camera.h"

#include <GLFW\glfw3.h> // Maybe in future create QuackKeyCodes to not include this here only for key codes

#include "Input.h"
#include "Event.h"
#include "MouseEvent.h"


namespace Quack
{
	Camera::Camera()
	{
		auto [mouseX, mouseY] = Input::GetMousePos();

		lastMousePos.x = mouseX;
		lastMousePos.y = mouseY;
	}

	void Camera::Update(float dt)
	{
		const glm::vec2& mousePos{ Input::GetMousePosX(), Input::GetMousePosY() };

		glm::vec2 offset = { mousePos.x - lastMousePos.x, lastMousePos.y - mousePos.y };;
		lastMousePos = mousePos;

		if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
		{
			fov = 45.f;
			MouseRotate(offset);

			float cameraSpeed = speed * dt;

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
		}

		else if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
			MousePan(offset, dt);
		// LMB + ALT rotate around front? Around world center?
	}

	void Camera::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<MouseScrolledEvent>([this](const MouseScrolledEvent& e)
		{
			if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
				MouseCameraSpeed(e.offsetY);
			else
				MouseZoom(e.offsetY);
		});
	}

	void Camera::MousePan(const glm::vec2& offset, float dt)
	{
		position -= glm::vec3(offset * dt, 0.f);
	}

	void Camera::MouseRotate(const glm::vec2& offset)
	{
		yaw += offset.x * rotationSpeed;
		pitch += offset.y * rotationSpeed;

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

	void Camera::MouseZoom(float offsetY)
	{
		fov -= offsetY;
		fov = fov < 1.f ? 1.f : fov;
	}

	void Camera::MouseCameraSpeed(float offsetY)
	{
		speed += offsetY;
		speed = speed < 1.f ? 1.f : speed;
	}
}
