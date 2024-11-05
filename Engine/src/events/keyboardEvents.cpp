#include "events/keyboardEvents.h"

namespace Engine
{
	KeyPressedEvent::KeyPressedEvent(const int key)
		: m_key(key)
	{ 
	} 

	int KeyPressedEvent::GetKey() const
	{
		return m_key;
	} 

	EventType KeyPressedEvent::GetStaticType()
	{
		return EventType::KeyPressed;
	} 

	EventType KeyPressedEvent::GetEventType() const
	{
		return GetStaticType();
	} 

	const char* KeyPressedEvent::GetName() const
	{
		return "KeyPressedEvent";
	}

	int KeyPressedEvent::GetCategoryFlags() const
	{
		return (EventCategoryKeyboard | EventCategoryInput);
	}

	KeyReleasedEvent::KeyReleasedEvent(const int key) 
		: m_key(key)
	{
	} 

	int KeyReleasedEvent::GetKey() const
	{
		return m_key;
	}

	EventType KeyReleasedEvent::GetStaticType()
	{
		return EventType::KeyReleased;
	}

	EventType KeyReleasedEvent::GetEventType() const
	{
		return GetStaticType();
	}

	const char* KeyReleasedEvent::GetName() const
	{
		return "KeyReleasedEvent";
	}

	int KeyReleasedEvent::GetCategoryFlags() const
	{
		return (EventCategoryKeyboard | EventCategoryInput);
	}
}