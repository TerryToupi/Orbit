#pragma once  
#include "core/logs.h" 
#include "window/window.h" 
#include "events/appEvents.h" 
#include "core/layerManager.h"
#include "renderer/device.h" 
#include "core/timeManager.h" 
#include "utilities/jobDispathcer.h"

int main(int argc, char** argv);

namespace Engine
{
	class Application
	{
	public: 
		Application(); 
		virtual ~Application();  

		void PushLayer(Layer* l);
		void PushOverlay(Layer* l);
		void RemoveLayer(Layer* l); 
		void RemoveOverlay(Layer* l);

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
		uint64_t m_time_elapsed;
		LayerManager m_layers;   
	
	private:
		static Application* s_Instance;
	}; 

	Application* CreateApplication();
} 
