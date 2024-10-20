#pragma once 

#include <events/event.h> 

namespace Engine
{
	class KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent(const int key);
		
		int GetKey() const;

		static	EventType GetStaticType();
		virtual	EventType GetEventType() const override;
		virtual const char* GetName() const override;
		virtual int GetCategoryFlags() const override; 

	private:
		int m_key;
	};

	class KeyReleasedEvent : public Event
	{
	public:
		KeyReleasedEvent(const int key);

		int GetKey() const;

		static	EventType GetStaticType();
		virtual	EventType GetEventType() const override;
		virtual const char* GetName() const override;
		virtual int GetCategoryFlags() const override;

	private:
		int m_key;
	};
}
