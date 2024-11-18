#pragma once   

#include <src/core/logs.h> 
#include <src/core/application.h> 

extern Engine::Application* Engine::CreateApplication();

int main(int argc, char** argv)
{
	Engine::Logging::Init();  

	auto app = Engine::CreateApplication(); 

	app->Run(); 

	delete app;
}

