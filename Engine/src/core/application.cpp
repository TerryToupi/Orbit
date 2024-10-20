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
		m_window->SetEventCallback(BIND_EVENT(Application::OnEvent));

		m_running = true; 

		#ifdef EDITOR_APPLICTATION 
		#endif
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
		while (m_running)
		{
			m_window->Update(); 

			#ifdef EDITOR_APPLICTATION 
			{ 
				ENGINE_CORE_TRACE("APPLICATION HOOKED");
			}
			#endif
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{ 
		m_running = false; 
		return true;
	} 

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{  
		ENGINE_CORE_TRACE("Width: {0}, Height: {1}", m_window->GetWidth(), m_window->GetHeight());
		return true;
	}


	void Application::OnEvent(Event& e)
	{ 
		EventDispatcher dispatcher(e); 
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT(Application::OnWindowResize));
	}
}

