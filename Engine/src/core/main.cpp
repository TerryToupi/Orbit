#include <iostream>    
#include <core/logs.h>

int main(void)
{  
    Engine::Logging::Init();
    ENGINE_CORE_INFO("have a nice day!");
    return 0;
}