#include <window/windowAPI.h>  
#include <core/assert.h>

namespace Engine
{ 
	static uint8_t s_activeWindowCount = 0;

	WindowAPI::WindowAPI(const WindowConfig& config) 
	{  
		m_width = config.Width;
		m_height = config.Height;  
		m_windowName = config.WindowName;

		if (s_activeWindowCount == 0)
		{
			int status = glfwInit(); 
			ENGINE_ASSERT(status);
		} 

		m_nativeWindow = glfwCreateWindow((int)m_width, (int)m_height, m_windowName.c_str(), nullptr, nullptr);
		++s_activeWindowCount; 
	} 

	WindowAPI::~WindowAPI()
	{
	} 

	unsigned int WindowAPI::GetWidth() const
	{
		return m_width;
	} 

	unsigned int WindowAPI::GetHeight() const
	{
		return m_height;
	} 

	void* WindowAPI::GetNativeWindow() const
	{
		return m_nativeWindow;
	} 

	void WindowAPI::Update() const
	{
	}

	void WindowAPI::ResizeWindow(int w, int h) const
	{
	}
}

