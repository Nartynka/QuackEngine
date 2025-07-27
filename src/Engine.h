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

	class Engine
	{
	public:
		Engine();
		~Engine();

		void OnEvent(Event& event);

		void Run();

		void OnLeftMouseButton(const MouseLeftButtonPressedEvent& e);
		void OnRightMouseButton(const MouseRightButtonPressedEvent& e);
		void ProcessInput(float dt); // temporary, input pooling soon
	private:
		std::unique_ptr<Window> window;
		std::unique_ptr<Renderer> renderer;
		Scene* scene; // @TODO: Maybe change to a smart ptr

		//bool bIsRunning = true;
	};
}
