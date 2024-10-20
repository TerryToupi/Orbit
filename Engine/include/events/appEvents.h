#pragma once 

#include <events/event.h> 

namespace Engine
{
	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;

		static	EventType GetStaticType();
		virtual	EventType GetEventType() const override;
		virtual const char* GetName() const override; 
		virtual int GetCategoryFlags() const override;
	}; 

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height);

		unsigned int GetWidth() const;
		unsigned int GetHeight() const;

		static	EventType GetStaticType();
		virtual	EventType GetEventType() const override;
		virtual const char* GetName() const override; 
		virtual int GetCategoryFlags() const override;
	private:
		unsigned int m_Width, m_Height;
	};
}
