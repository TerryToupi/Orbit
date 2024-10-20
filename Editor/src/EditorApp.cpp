#include <MainEntry.h>   

namespace Engine
{
	class EnditorApp : public Application
	{

	}; 

	Application* CreateApplication()
	{ 
		ENGINE_CLIENT_TRACE("Instanciating Application");
		return new EnditorApp();
	}
}