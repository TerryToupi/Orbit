#pragma once   
#include "src/core/core.h"
#include "src/events/event.h"

namespace Engine
{
	struct WindowConfig
	{ 
		std::string WindowName;
		uint32_t Width;
		uint32_t Height; 

		WindowConfig(const std::string& title = "Orbit",
				uint32_t Width = 1600,
				uint32_t Height = 900 
			) : WindowName(title), Width(Width), Height(Height)
		{
		}
	}; 

	class Window
	{  
	public:  
		using EventCallback = std::function<void(Event&)>; 

		static inline Window* instance = nullptr;    

		virtual ~Window() = default;

		virtual void Init() = 0;
		virtual void ShutDown() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Update(uint64_t ts) = 0;
		virtual void ResizeWindow(int w, int h) const = 0;  

		virtual void SetEventCallback(const EventCallback& cb) = 0;
	};
}
