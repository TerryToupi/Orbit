#pragma once  
#include <core/logs.h>  
#include <window/window.h> 
#include <events/appEvents.h>

int main(int argc, char** argv);

namespace Engine
{
	class Application
	{
	public: 
		Application(); 
		virtual ~Application(); 
		
		static Application& Get();  

	private:
		void Run(); 
		bool OnWindowClose(WindowCloseEvent& e); 
		bool OnWindowResize(WindowResizeEvent& e);
		void OnEvent(Event& e);

	private: 
		friend int ::main(int argc, char** argv); 

	private: 
		bool m_running = false;
		Scope<Window> m_window; 
	
	private:
		static Application* s_Instance;
	}; 

	Application* CreateApplication();
} 
