#pragma once

#include <memory>

#include "Event.h"
#include "MouseEvent.h"
#include "KeyEvent.h"

namespace Quack
{
	class Window;
	class Scene;
	class Camera;
	class UI;

	class Engine
	{
	public:
		Engine();
		~Engine();

		void OnEvent(Event& event);

		void Run();

		void OnMouseButtonPressed(const MouseButtonPressedEvent& e);
	private:
		std::unique_ptr<Window> window;
		std::shared_ptr<Scene> scene;
		std::unique_ptr<Camera> camera;
		std::unique_ptr<UI> ui;
		//bool bIsRunning = true;
	};
}
