#include <renderer/device.h>

#ifdef VULKAN_BACKEND
#include <platform/Vulkan/VulkanDevice.h>
#endif

namespace Engine
{  
	Device* Device::s_instance = nullptr;

	Device& Device::Get()
	{ 
		return *s_instance;
	}

	void Device::Create()
	{ 
		ENGINE_ASSERT(!Device::s_instance); 
		
		#ifdef VULKAN_BACKEND
		s_instance = new VulkanDevice();
		s_instance->Init(); 
		#else 
		ENGINE_CORE_ERROR("No backend definition found! Please Select: VULKAN_BACKEND, ...");
		#endif
	} 

	void Device::Destroy()
	{  
		ENGINE_ASSERT(Device::s_instance);
		s_instance->ShutDown(); 
		delete s_instance;
	}
}