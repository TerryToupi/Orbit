#include "events/mouseEvents.h"

namespace Engine
{
	MouseMotionEvent::MouseMotionEvent(const float x, const float y) 
		: m_x(x), m_y(y)
	{
	} 

	float MouseMotionEvent::GetX() const
	{
		return m_x;
	} 

	float MouseMotionEvent::GetY() const
	{
		return m_y;
	} 

	EventType MouseMotionEvent::GetStaticType()
	{
		return EventType::MouseMoved;
	} 

	EventType MouseMotionEvent::GetEventType() const
	{
		return GetStaticType();
	} 

	const char* MouseMotionEvent::GetName() const
	{
		return "MouseMotion";
	}

	int MouseMotionEvent::GetCategoryFlags() const
	{
		return (EventCategoryMouse | EventCategoryInput);
	} 

	MouseScrolledEvent::MouseScrolledEvent(const float xOffset, const float yOffset) 
		: m_XOffset(xOffset), m_YOffset(yOffset)
	{
	} 

	float MouseScrolledEvent::GetXOffset() const
	{
		return m_XOffset;
	} 

	float MouseScrolledEvent::GetYOffset() const
	{
		return m_YOffset;
	} 

	EventType MouseScrolledEvent::GetStaticType()
	{
		return EventType::MouseScrolled;
	} 

	EventType MouseScrolledEvent::GetEventType() const
	{
		return GetStaticType();
	}

	const char* MouseScrolledEvent::GetName() const
	{
		return "MouseScrolledEvent";
	}

	int MouseScrolledEvent::GetCategoryFlags() const
	{
		return (EventCategoryMouse | EventCategoryInput);
	} 

	MouseButtonPressedEvent::MouseButtonPressedEvent(int button) 
		: m_Button(button)
	{
	} 

	int MouseButtonPressedEvent::GetMouseButton() const
	{
		return m_Button;
	} 

	EventType MouseButtonPressedEvent::GetStaticType()
	{
		return EventType::MouseButtonPressed;
	}

	EventType MouseButtonPressedEvent::GetEventType() const
	{
		return GetStaticType();
	}

	const char* MouseButtonPressedEvent::GetName() const
	{
		return "MouseButtonPressedEvent";
	}

	int MouseButtonPressedEvent::GetCategoryFlags() const
	{
		return (EventCategoryMouse | EventCategoryInput);
	}

	MouseButtonReleasedEvent::MouseButtonReleasedEvent(int button) 
		: m_Button(button)
	{
	}

	int MouseButtonReleasedEvent::GetMouseButton() const
	{
		return m_Button;
	}

	EventType MouseButtonReleasedEvent::GetStaticType()
	{
		return EventType::MouseButtonReleased;
	}

	EventType MouseButtonReleasedEvent::GetEventType() const
	{
		return GetStaticType();
	}

	const char* MouseButtonReleasedEvent::GetName() const
	{
		return "MouseButtonReleasedEvent";
	}

	int MouseButtonReleasedEvent::GetCategoryFlags() const
	{
		return (EventCategoryMouse | EventCategoryInput);
	}
}