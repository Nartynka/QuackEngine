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

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float posX, float posY) 
			: Event(), posX(posX), posY(posY)
		{
		}

		const char* GetName() const override
		{
			return "Mouse moved event";
		}

		EventType GetEventType() const override
		{
			return EventType::MouseMoved;
		}

		static EventType GetStaticEventType()
		{
			return EventType::MouseMoved;
		}

	private:
		float posX, posY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float offsetX, float offsetY) 
			: Event(), offsetX(offsetX), offsetY(offsetY)
		{
		}

		const char* GetName() const override
		{
			return "Mouse move event";
		}

		EventType GetEventType() const override
		{
			return EventType::MouseScrolled;
		}

		static EventType GetStaticEventType()
		{
			return EventType::MouseScrolled;
		}

	private:
		float offsetX, offsetY;
	};
}