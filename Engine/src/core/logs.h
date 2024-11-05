#pragma once 

#pragma warning(push, 0)   
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#pragma warning(pop)  

#include	<memory>

namespace Engine
{  
	class Logging
	{
	public:
		static void Init(); 
		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; } 

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
} 

#ifdef DEBUG 
#define ENGINE_CORE_TRACE(...)		::Engine::Logging::GetCoreLogger()->trace(__VA_ARGS__)
#define ENGINE_CORE_INFO(...)		::Engine::Logging::GetCoreLogger()->info(__VA_ARGS__)
#define ENGINE_CORE_WARN(...)		::Engine::Logging::GetCoreLogger()->warn(__VA_ARGS__)
#define ENGINE_CORE_ERROR(...)		::Engine::Logging::GetCoreLogger()->error(__VA_ARGS__)

#define ENGINE_CLIENT_TRACE(...)	::Engine::Logging::GetClientLogger()->trace(__VA_ARGS__)
#define ENGINE_CLIENT_INFO(...)		::Engine::Logging::GetClientLogger()->info(__VA_ARGS__)
#define ENGINE_CLIENT_WARN(...)		::Engine::Logging::GetClientLogger()->warn(__VA_ARGS__)
#define ENGINE_CLIENT_ERROR(...)	::Engine::Logging::GetClientLogger()->error(__VA_ARGS__)
#endif 

#ifdef RELEASE 
#define ENGINE_CORE_TRACE(...)		
#define ENGINE_CORE_INFO(...)		
#define ENGINE_CORE_WARN(...)		
#define ENGINE_CORE_ERROR(...)		

#define ENGINE_CLIENT_TRACE(...)		
#define ENGINE_CLIENT_INFO(...)		
#define ENGINE_CLIENT_WARN(...)		
#define ENGINE_CLIENT_ERROR(...)		
#endif  

#ifdef DIST 
#define ENGINE_CORE_TRACE(...)		
#define ENGINE_CORE_INFO(...)		
#define ENGINE_CORE_WARN(...)		
#define ENGINE_CORE_ERROR(...)		

#define ENGINE_CLIENT_TRACE(...)		
#define ENGINE_CLIENT_INFO(...)		
#define ENGINE_CLIENT_WARN(...)		
#define ENGINE_CLIENT_ERROR(...)		
#endif 

