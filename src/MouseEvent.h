#pragma once

#include "Event.h"

namespace Quack
{
	class MouseButtonPressedEvent : public Event
	{
	public:
		MouseButtonPressedEvent(int button) : Event(), button(button)
		{
		}

		const char* GetName() const override
		{
			return "Mouse button pressed event";
		}

		int GetButton() const
		{
			return button;
		}

		EventType GetEventType() const override
		{
			return EventType::MouseButtonPressed;
		}

		static EventType GetStaticEventType()
		{
			return EventType::MouseButtonPressed;
		}

	private:
		int button;
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

		float offsetX, offsetY;
	private:
	};
}