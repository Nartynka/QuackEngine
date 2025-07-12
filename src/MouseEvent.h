#pragma once

#include "Event.h"

namespace Quack
{
	class MouseLeftButtonPressedEvent : public Event
	{
	public:
		MouseLeftButtonPressedEvent() : Event()
		{
		}

		const char* GetName() const override
		{
			return "Mouse left button pressed event";
		}

		EventType GetEventType() const override
		{
			return EventType::MouseLeftButtonPressed;
		}

		static EventType GetStaticEventType()
		{
			return EventType::MouseLeftButtonPressed;
		}
	};

	class MouseRightButtonPressedEvent : public Event
	{
	public:
		MouseRightButtonPressedEvent() : Event()
		{
		}

		const char* GetName() const override
		{
			return "Mouse right button pressed event";
		}

		EventType GetEventType() const override
		{
			return EventType::MouseRightButtonPressed;
		}

		static EventType GetStaticEventType()
		{
			return EventType::MouseRightButtonPressed;
		}
	};
}