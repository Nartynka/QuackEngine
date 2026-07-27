#pragma once

namespace Quack
{
	class Window;
	class Scene;

	class UI
	{
	public:
		UI(Window* window, Scene* scene);
		~UI();
		
		void SetContext(Scene* scene);

		void StartFrame();
		void EndFrame();

		void Shutdown();
	private:

		void RenderMenu();
		void RenderStats();

		void RenderCombo(const char* label, const char* elements[], int count, int& currentIdx);

		Scene* context;
	};
}