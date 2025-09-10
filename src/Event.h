#pragma once
#include <functional>
#include <vector>
#include "Log.h"

namespace Quack
{
	enum class EventType
	{
		None = 0,
		//WindowResize, WindowClose,
		KeyPressed, //KeyReleased,
		MouseLeftButtonPressed, MouseRightButtonPressed, MouseMoved, MouseScrolled //MouseButtonReleased, 
	};

	class Event
	{
	public:
		bool handled = false;

 		virtual const char* GetName() const = 0; // For debugging/logging
		virtual EventType GetEventType() const = 0;
	};


	class EventDispatcher
	{
		template<typename T>
		using EventCallback = std::function<void(const T&)>;
	public:

		EventDispatcher(Event& event) : event(event) 
		{
		}

		template<typename T>
		void Dispatch(EventCallback<T> callback)
		{
			if (event.GetEventType() == T::GetStaticEventType() && !event.handled)
			{
				//QUACK_LOG("Event!!! {}", event.GetName());
				callback((const T&)event);
				event.handled = true;
			}
		}

	private:
		Event& event;
	};

}