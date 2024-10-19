#pragma once   
#include <core/core.h>

namespace Engine
{
	struct WindowConfig
	{ 
		std::string WindowName;
		uint32_t Width;
		uint32_t Height; 

		WindowConfig(const std::string& title = "Orbit",
				uint32_t Width = 1024,
				uint32_t Height = 720
			) : WindowName(title), Width(Width), Height(Height)
		{
		}
	}; 

	class Window
	{  
	public: 
		using EventCallback = std::function<void(void)>;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Update() const = 0;
		virtual void ResizeWindow(int w, int h) const = 0; 

		virtual void* GetNativeWindow() const = 0; 

		static Scope<Window> Create(const WindowConfig& config);
	};
}
