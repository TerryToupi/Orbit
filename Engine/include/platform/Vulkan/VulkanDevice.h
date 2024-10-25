#pragma once 
#include <renderer/device.h>  

// Vulkan
#include <platform/Vulkan/VulkanCore.h> 
#include <platform/Vulkan/VulkanWindow.h>

namespace Engine
{ 
	class VulkanDevice : public Device 
	{  
	public:
		const VkDevice& GetDevice();

	protected:
		virtual void Init() override;
		virtual void ShutDown() override;  

	private: 
		void CreateInstance();
		bool CheckValidationLayerSupport(); 
		std::vector<const char*> GetRequiredExtensions();
		
	private:  
		VkDevice m_device;
		VkInstance m_instance; 

		const std::vector<const char*> m_validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};  

		#ifdef DEBUG
		const bool m_enableValidationLayers = true;
		#elif RELEASE
		const bool m_enableValidationLayers = false; 
		#elif DIST
		const bool m_enableValidationLayers = false; 
		#endif
	};
}
