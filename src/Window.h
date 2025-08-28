#pragma once

#include <memory>
#include <functional>

#include "Event.h"

struct GLFWwindow;

namespace Quack
{
	class UI;

	class Window
	{
		using EventCallback = std::function<void(Event&)>;
	public:
		Window(unsigned int width, unsigned int height);
		~Window();
		
		void Update();

		inline GLFWwindow* GetWindow() const { return window; }
		inline UI* GetUI() const { return &(*ui); } // @TODO: delete this xD

		void Shutdown();
		void Init(unsigned int width, unsigned int height);

		void SetCallback(const EventCallback& callback) { data.callback = callback; }
	private:

		GLFWwindow* window;

		unsigned int width;
		unsigned int height;

		std::unique_ptr<UI> ui;

		struct WindowData
		{
			EventCallback callback;
		};

		WindowData data;
	};
}