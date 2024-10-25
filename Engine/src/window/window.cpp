#include <window/window.h> 

#include <platform/Vulkan/VulkanWindow.h>

namespace Engine
{
	Unique<Window> Window::Create(const WindowConfig& config)
	{ 
	#ifdef VULKAN_BACKEND
		return MakeUnique<VulkanWindow>(config); 
	#endif
	}
}
