#pragma once

#include <memory>
#include <functional>

#include "Event.h"

struct GLFWwindow;

namespace Quack
{
	class Window
	{
		using EventCallback = std::function<void(Event&)>;
	public:
		Window(unsigned int width, unsigned int height);
		~Window();
		
		void Update();

		inline GLFWwindow* GetWindow() const { return window; }

		void Shutdown();
		void Init(unsigned int width, unsigned int height);

		void SetCallback(const EventCallback& callback) { data.callback = callback; }
	private:

		GLFWwindow* window;

		unsigned int width;
		unsigned int height;

		struct WindowData
		{
			EventCallback callback;
		};

		WindowData data;
	};
}