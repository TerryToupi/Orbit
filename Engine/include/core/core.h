#pragma once 

#include <memory>  
#include <functional>
#include <string>

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

namespace Engine
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ...Args>
	constexpr Scope<T> MakeScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ...Args>
	constexpr Ref<T> MakeRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}
