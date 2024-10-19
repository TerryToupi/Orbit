#include <core/application.h>
#include <core/assert.h>

namespace Engine
{ 
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{   
		ENGINE_ASSERT_MSG(!s_Instance, "Application instance already exists!");
		s_Instance = this;  

		ENGINE_CORE_INFO("Initializing Window manager!"); 
		
		m_window = Window::Create(WindowConfig());

		m_running = true;
	}

	Application::~Application()
	{ 

	}

	Application& Application::Get()
	{
		return *s_Instance;
	}

	void Application::Run()
	{  
		ENGINE_CORE_INFO("Engine is alive!"); 

		while (m_running)
		{
			
		}
	}

	void Application::ShutDown()
	{ 

	} 

	void Application::Event()
	{ 

	}
}

