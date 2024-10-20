#pragma once 

#include <events/event.h> 

namespace Engine
{
	class MouseMotionEvent : public Event
	{
	public:
		MouseMotionEvent(const float x, const float y);

		float GetX() const;
		float GetY() const;   

		static	EventType GetStaticType();
		virtual	EventType GetEventType() const override;
		virtual const char* GetName() const override;
		virtual int GetCategoryFlags() const override; 

	private: 
		float m_x;
		float m_y;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset);

		float GetXOffset() const;
		float GetYOffset() const;

		static	EventType GetStaticType();
		virtual	EventType GetEventType() const override;
		virtual const char* GetName() const override;
		virtual int GetCategoryFlags() const override; 

	private:
		float m_XOffset, m_YOffset;
	}; 

	class MouseButtonPressedEvent : public Event
	{
	public: 
		MouseButtonPressedEvent(int button);

		int GetMouseButton() const; 

		static	EventType GetStaticType();
		virtual	EventType GetEventType() const override;
		virtual const char* GetName() const override;
		virtual int GetCategoryFlags() const override;
	private:
		int m_Button;
	}; 

	class MouseButtonReleasedEvent : public Event
	{
	public: 
		MouseButtonReleasedEvent(int button);

		int GetMouseButton() const;

		static	EventType GetStaticType();
		virtual	EventType GetEventType() const override;
		virtual const char* GetName() const override;
		virtual int GetCategoryFlags() const override;
	private:
		int m_Button;
	};
}
