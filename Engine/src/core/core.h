#pragma once 

#include <memory>
#include <functional>
#include <string>
#include <optional>
#include <map>

#ifdef DEBUG
	#if defined(OP_WINDOWS)
		#define DEBUGBREAK() __debugbreak() 
	#elif defined(OP_LINUX)
		#include <signal.h>
		#define DEBUGBREAK() raise(SIGTRAP)
	#elif defined(OP_MACOS)
		#define DEBUGBREAK() __builtin_debugtrap()
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif 

#else
	#define DEBUGBREAK()  

#endif   

#define BIT(x) 1<<x 

#define BIND_EVENT(function) [this](auto&&... args) -> decltype(auto) { return this->function(std::forward<decltype(args)>(args)...); } 

