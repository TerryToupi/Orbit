#pragma once 

#include "src/core/logs.h"
#include "src/core/core.h"
#include <filesystem>


#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#define ENGINE_EXPAND_MACRO(x) x
#define ENGINE_STRINGIFY_MACRO(expr) TO_STRING(expr)

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define ENGINE_INTERNAL_ASSERT_IMP(type, check, msg, ...) { if(!(check)) { ENGINE##type##ERROR(msg, __VA_ARGS__); DEBUGBREAK(); } }
#define ENGINE_INTERNAL_ASSERT_WITH_MSG(type, check, ...) ENGINE_INTERNAL_ASSERT_IMP(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define ENGINE_INTERNAL_ASSERT_NO_MSG(type, check) ENGINE_INTERNAL_ASSERT_IMP(type, check, "Assertion '{0}' failed at {1}:{2}", ENGINE_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define ENGINE_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define ENGINE_INTERNAL_ASSERT_GET_MACRO(...) ENGINE_EXPAND_MACRO( ENGINE_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, ENGINE_INTERNAL_ASSERT_WITH_MSG, ENGINE_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define ENGINE_ASSERT(...) ENGINE_EXPAND_MACRO( ENGINE_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
