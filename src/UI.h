#pragma once

namespace Quack
{
	class Window;

	class UI
	{
	public:
		UI(Window* window);
		~UI();
		
		void StartFrame();
		void EndFrame();

		void Shutdown();
		int entityCount = 0;
	private:
		void OnEvent();

	};
}