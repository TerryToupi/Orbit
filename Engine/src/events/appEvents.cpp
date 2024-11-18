#include "src/events/appEvents.h"

namespace Engine
{
	EventType WindowCloseEvent::GetStaticType()
	{
		return EventType::WindowClose;
	} 

	EventType WindowCloseEvent::GetEventType() const
	{
		return GetStaticType();
	} 

	const char* WindowCloseEvent::GetName() const
	{
		return "WindowCloseEvent";
	}

	int WindowCloseEvent::GetCategoryFlags() const
	{
		return EventCategoryApplication;
	} 

	WindowResizeEvent::WindowResizeEvent(unsigned int width, unsigned int height) 
		: m_Height(height), m_Width(width)
	{
	} 

	unsigned int WindowResizeEvent::GetWidth() const
	{
		return m_Width;
	} 

	unsigned int WindowResizeEvent::GetHeight() const
	{
		return m_Height;
	} 

	EventType WindowResizeEvent::GetStaticType()
	{
		return EventType::WindowResize;
	} 

	EventType WindowResizeEvent::GetEventType() const
	{
		return GetStaticType();
	} 

	const char* WindowResizeEvent::GetName() const
	{
		return "WindowResizeEvent";
	} 

	int WindowResizeEvent::GetCategoryFlags() const
	{
		return EventCategoryApplication;
	}
}