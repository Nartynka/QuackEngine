#pragma once

#include "Event.h"

namespace Quack
{
	class KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent(int keyCode) 
			: Event(), keyCode(keyCode)
		{
		}

		const char* GetName() const override
		{
			return "Key pressed event";
		}

		int GetKeyCode() const
		{
			return keyCode;
		}

		//const char* Print() const override
		//{
		//	return "Key pressed event, key: " + keyCode;
		//}

		EventType GetEventType() const override
		{
			return EventType::KeyPressed;
		}

		static EventType GetStaticEventType() 
		{
			return EventType::KeyPressed;
		}
		
	private:
		int keyCode;
	};
}