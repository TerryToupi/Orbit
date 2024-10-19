#pragma once 

#include <window/window.h>   
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Engine
{
	class WindowAPI : public Window
	{
	public:
		WindowAPI(const WindowConfig& config);
		virtual ~WindowAPI();

		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;

		virtual void Update() const override;
		virtual void ResizeWindow(int w, int h) const override;

		virtual void* GetNativeWindow() const override;

	private: 
		std::string m_windowName;
		uint32_t m_width; 
		uint32_t m_height;  

		EventCallback m_callBack;

		GLFWwindow* m_nativeWindow;
	};
}
