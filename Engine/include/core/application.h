#pragma once  
#include <core/logs.h>  
#include <window/window.h>

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
		void ShutDown();  
		void Event();

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
