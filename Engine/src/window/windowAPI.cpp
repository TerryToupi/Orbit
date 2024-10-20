#include <window/windowAPI.h>  
#include <core/assert.h> 
#include <events/appEvents.h>  
#include <events/keyboardEvents.h> 
#include <events/mouseEvents.h>

namespace Engine
{ 
	static uint8_t s_activeWindowCount = 0;

	WindowAPI::WindowAPI(const WindowConfig& config) 
	{  
		m_data.width = config.Width;
		m_data.height = config.Height;  
		m_windowName = config.WindowName;

		if (s_activeWindowCount == 0)
		{
			int status = glfwInit(); 
			ENGINE_ASSERT(status); 

		#ifdef VULKAN_BACKEND
			uint32_t extensionCount = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
			ENGINE_CORE_INFO("Vulkan Instance Extension Support: {0}", extensionCount);
		#endif
		} 

		m_nativeWindow = glfwCreateWindow((int)m_data.width, (int)m_data.height, m_windowName.c_str(), nullptr, nullptr);
		++s_activeWindowCount;

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

	WindowAPI::~WindowAPI()
	{ 
		glfwDestroyWindow(m_nativeWindow);
		--s_activeWindowCount;  

		ENGINE_CORE_INFO("Active window count: {0}", s_activeWindowCount);

		if (s_activeWindowCount == 0)
		{ 
			ENGINE_CORE_INFO("Terminating window API!");
			glfwTerminate();
		}
	} 

	unsigned int WindowAPI::GetWidth() const
	{
		return m_data.width;
	} 

	unsigned int WindowAPI::GetHeight() const
	{
		return m_data.height;
	} 

	void* WindowAPI::GetNativeWindow() const
	{
		return m_nativeWindow;
	} 

	void WindowAPI::Update() const
	{ 
		glfwPollEvents();
	}

	void WindowAPI::ResizeWindow(int w, int h) const
	{
	} 

	void WindowAPI::SetEventCallback(const EventCallback& cb)
	{ 
		m_data.callBack = cb;
	}
}

