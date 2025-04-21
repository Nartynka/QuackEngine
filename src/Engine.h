#pragma once

#include "window.h"

#include <memory>

namespace Quack
{
	class Engine
	{
	public:
		Engine();
		~Engine();

		void Run();
	private:
		std::unique_ptr<Window> window;
		//bool bIsRunning = true;
	};
}