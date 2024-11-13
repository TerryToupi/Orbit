#pragma once 

#include "window/window.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h" 

#define M_INPUT_PERIOD_US 500

namespace Engine
{
	class VulkanWindow final: public Window
	{
	public:
		VulkanWindow(const WindowConfig& config);
		virtual ~VulkanWindow() = default;
 
		virtual void Init() override;
		virtual void ShutDown() override;

		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;

		virtual void Update(uint64_t ts) override;
		virtual void ResizeWindow(int w, int h) const override;

		virtual void SetEventCallback(const EventCallback& cb) override;

		GLFWwindow* GetNativeWindow() const;

	private:
		struct WindowData
		{
			uint32_t width;
			uint32_t height;

			EventCallback callBack;
		} m_data;

		std::string m_windowName;

		GLFWwindow* m_nativeWindow; 

		uint64_t m_time_elapsed;
	};
}
