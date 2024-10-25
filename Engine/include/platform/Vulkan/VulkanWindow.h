#pragma once 

#include <window/window.h>
#include <GLFW/glfw3.h>

namespace Engine
{
	class VulkanWindow : public Window
	{
	public:
		VulkanWindow(const WindowConfig& config);
		virtual ~VulkanWindow();

		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;

		virtual void Update() const override;
		virtual void ResizeWindow(int w, int h) const override;

		virtual void SetEventCallback(const EventCallback& cb) override;

		virtual void* GetNativeWindow() const override;

	private:
		struct WindowData
		{
			uint32_t width;
			uint32_t height;

			EventCallback callBack;
		} m_data;

		std::string m_windowName;

		GLFWwindow* m_nativeWindow;
	};
}
