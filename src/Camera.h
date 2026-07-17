#pragma once

#include <glm.hpp>

namespace Quack
{
	class Event;

	class Camera
	{
	public:
		Camera();

		glm::vec3 position = glm::vec3(0.f, 0.5f, 5.f);
		glm::vec3 front = glm::vec3(0.f, 0.f, -1.f);
		glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
		float fov = 45.0f;

		void Update(float dt);
		void OnEvent(Event& event);

		glm::mat4 GetView();
		glm::mat4 GetProjection(int width, int height);
	private:
		float speed = 5.f;
		float rotationSpeed = 0.1f;

		float yaw = -90.f;
		float pitch = 0.f;

		glm::vec2 lastMousePos;

		void MousePan(const glm::vec2& offset, float dt);
		void MouseRotate(const glm::vec2& offset);
		void MouseZoom(float offsetY);
		void MouseCameraSpeed(float offsetY);
	};
}