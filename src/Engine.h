#pragma once

#include <memory>

#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"

namespace Quack
{
	class Window;
	class Renderer;
	class Scene;
	class Camera;

	class Engine
	{
	public:
		Engine();
		~Engine();

		void OnEvent(Event& event);

		void Run();

		void OnLeftMouseButton(const MouseLeftButtonPressedEvent& e);
		void OnRightMouseButton(const MouseRightButtonPressedEvent& e);
	private:
		std::unique_ptr<Window> window;
		std::unique_ptr<Renderer> renderer;
		std::unique_ptr<Scene> scene;
		std::unique_ptr<Camera> camera;
		//bool bIsRunning = true;
	};
}
