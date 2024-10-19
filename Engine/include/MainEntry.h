#pragma once  

#include <core/logs.h> 
#include <core/application.h>

extern Engine::Application* Engine::CreateApplication();

#ifdef OP_WINDOWS 

int main(int argc, char** argv)
{
	Engine::Logging::Init();  

	auto app = Engine::CreateApplication(); 

	app->Run(); 

	delete app;
}

#endif
