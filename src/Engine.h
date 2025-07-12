#pragma once

#include <memory>

#include "Event.h"
#include "MouseEvent.h"

namespace Quack
{
	class Window;
	class Renderer;

	class Engine
	{
	public:
		Engine();
		~Engine();

		void OnEvent(Event& event);

		void Run();

		void OnMouseButton(const MouseLeftButtonPressedEvent& e);
	private:
		std::unique_ptr<Window> window;
		std::unique_ptr<Renderer> renderer;
		//bool bIsRunning = true;
	};
}
