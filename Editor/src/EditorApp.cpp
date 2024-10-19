#include <MainEntry.h> 

namespace Engine
{
	class EnditorApp : public Application
	{

	}; 

	Application* CreateApplication()
	{
		return new EnditorApp();
	}
}