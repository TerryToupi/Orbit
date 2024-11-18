#pragma once 

#include "src/core/logs.h"
#include "src/core/core.h"
#include <filesystem>

#define ASSERT(type, check, msg, ...) { if (!check) { ENGINE##type##ERROR(msg, __VA_ARGS__); DEBUGBREAK(); } }

#define ENGINE_ASSERT_MSG(check, ...) ASSERT(_CORE_, check, "Assertion failed: {0}", __VA_ARGS__) 
#define ENGINE_ASSERT(check) ASSERT(_CORE_, check, "Assertion '{0}' failed at '{1}':{2}", #check, std::filesystem::path(__FILE__).filename().string(), __LINE__)  

#define CLIENT_ASSERT_MSG(check, ...) ASSERT(_CLIENT_, check, "Assertion failed: {0}", __VA_ARGS__) 
#define CLIENT_ASSERT(check) ASSERT(_CLIENT_, check, "Assertion '{0}' failed at '{1}':{2}", #check, std::filesystem::path(__FILE__).filename().string(), __LINE__)
