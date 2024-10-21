#include <MainEntry.h>    
#include <EditorLayer.h>

namespace Editor 
{
	class Editor: public Engine::Application
	{
	public:   
		Editor()
		{  
			PushLayer(new EditorLayer());
		}

	private: 
	}; 

} 

Engine::Application* Engine::CreateApplication()
{ 
	ENGINE_CLIENT_TRACE("Instanciating Application");
	return new Editor::Editor();
}