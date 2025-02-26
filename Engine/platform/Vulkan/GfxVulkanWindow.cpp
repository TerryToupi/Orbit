#include "platform/Vulkan/GfxVulkanWindow.h"
#include "src/core/assert.h" 
#include "src/events/appEvents.h" 
#include "src/events/keyboardEvents.h"
#include "src/events/mouseEvents.h"
#include "src/core/timeManager.h"

namespace Engine
{
	VulkanWindow::VulkanWindow(const WindowConfig& config)
		: m_data{
			.width = config.Width,
			.height = config.Height,
			.callBack = nullptr
		},
		m_windowName(config.WindowName),
		m_nativeWindow(nullptr)
	{
	}

	void VulkanWindow::ShutDown()
	{
		glfwDestroyWindow(m_nativeWindow);
		glfwTerminate();
	}

	void VulkanWindow::Init()
	{ 
		int status = glfwInit();
		ENGINE_ASSERT(status);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		m_nativeWindow = glfwCreateWindow((int)m_data.width, (int)m_data.height, m_windowName.c_str(), nullptr, nullptr);

		glfwSetWindowUserPointer(m_nativeWindow, &m_data);

		glfwSetWindowCloseCallback(m_nativeWindow, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.callBack(event);
			});

		glfwSetWindowSizeCallback(m_nativeWindow, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.width = width;
				data.height = height;

				WindowResizeEvent event(width, height);
				data.callBack(event);
			});

		glfwSetKeyCallback(m_nativeWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key);
					data.callBack(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.callBack(event);
					break;
				}
				}
			});

		glfwSetCursorPosCallback(m_nativeWindow, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMotionEvent event((float)xPos, (float)yPos);
				data.callBack(event); 
			});

		glfwSetScrollCallback(m_nativeWindow, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.callBack(event);
			});

		glfwSetMouseButtonCallback(m_nativeWindow, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.callBack(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.callBack(event);
					break;
				}
				}
			});
	}

	unsigned int VulkanWindow::GetWidth() const
	{
		return m_data.width;
	}

	unsigned int VulkanWindow::GetHeight() const
	{
		return m_data.height;
	}

	GLFWwindow* VulkanWindow::GetNativeWindow() const 
	{
		return m_nativeWindow;
	}

	void VulkanWindow::Update(uint64_t ts)
	{ 
		glfwPollEvents(); 
		glfwSwapBuffers(m_nativeWindow);
	}

	void VulkanWindow::ResizeWindow(int w, int h) const
	{
	}

	void VulkanWindow::SetEventCallback(const EventCallback& cb)
	{
		m_data.callBack = cb;
	}
}

