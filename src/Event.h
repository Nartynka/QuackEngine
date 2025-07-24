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
		MouseLeftButtonPressed, MouseRightButtonPressed, //MouseButtonReleased, MouseMove, MouseScrolled
	};

	class Event
	{
	public:
		//~Event() = default;
		//Event() = default;

 		virtual const char* GetName() const = 0; // For debugging/logging
		virtual EventType GetEventType() const = 0;
	protected:
		bool handled = false;
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
			if (event.GetEventType() == T::GetStaticEventType())
			{
				//QUACK_LOG("Event!!! {}", event.GetName());
				callback((const T&)event);
			}
		}

	private:
		Event& event;
	};

}