#include "MainEntry.h"

class GameApp: public Engine::Application
{
public:   
    GameApp()
    {  
        // PushLayer(new EditorLayer());
    }

private: 
}; 

Engine::Application* Engine::CreateApplication()
{  
	return new GameApp();
}