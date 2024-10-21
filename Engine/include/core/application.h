#pragma once  
#include <core/logs.h>  
#include <window/window.h> 
#include <events/appEvents.h> 
#include <core/layerManager.h> 
#include <editorLayer/editorBackend.h>

int main(int argc, char** argv);

namespace Engine
{
	class Application
	{
	public: 
		Application(); 
		virtual ~Application(); 

		Window& GetWindow();  

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
		Scope<Window> m_window;
		LayerManager m_layers;   
		EditorBackend* m_editor;
	
	private:
		static Application* s_Instance;
	}; 

	Application* CreateApplication();
} 
