#pragma once

#include <memory>

namespace Quack
{
	class Window;
	class Renderer;

	class Engine
	{
	public:
		Engine();
		~Engine();

		void Run();
	private:
		std::unique_ptr<Window> window;
		std::unique_ptr<Renderer> renderer;
		//bool bIsRunning = true;
	};
}
