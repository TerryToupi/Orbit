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
		
		m_editor = nullptr;
		#ifdef EDITOR_APPLICATION 
		ENGINE_CORE_INFO("Editor application initialization!");
		m_editor = new EditorBackend(); 
		PushOverlay(m_editor);
		#endif
	}

	Application::~Application()
	{ 

	}

	Application& Application::Get()
	{
		return *s_Instance;
	}

	Window& Application::GetWindow()
	{ 
		return *m_window;
	}

	void Application::PushLayer(Layer* l)
	{ 
		m_layers.PushLayer(l);
	} 

	void Application::PushOverlay(Layer* l)
	{ 
		m_layers.PushOverlay(l);
	}

	void Application::RemoveLayer(Layer* l)
	{ 
		m_layers.PopLayer(l);
	}

	void Application::RemoveOverlay(Layer* l)
	{ 
		m_layers.PopOverlay(l);
	}

	void Application::Run()
	{  
		while (m_running)
		{
			m_window->Update();

			for (auto layer = m_layers.begin(); layer != m_layers.end(); layer++)
			{
				(*layer)->OnUpdate();
			}

			#ifdef EDITOR_APPLICATION 
			{ 
				m_editor->start(); 
				for (auto layer = m_layers.begin(); layer != m_layers.end(); layer++)
				{
					(*layer)->OnEditorRender();
				} 
				m_editor->finish();
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

		for (auto layer = m_layers.rbegin(); layer != m_layers.rend(); layer++)
		{
			if (e.Handled) break; 
			(*layer)->OnEvent(e);
		}
	}
}

