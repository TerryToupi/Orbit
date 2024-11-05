#pragma once 
#include "renderer/device.h"

// Vulkan
#include "platform/Vulkan/VulkanCore.h" 
#include "platform/Vulkan/VulkanWindow.h"

namespace Engine
{  
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	}; 

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities{};
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	class VulkanDevice final: public Device 
	{  
	public:
		const VkDevice& GetDevice(); 
		const VkInstance& GetInsance(); 
		const VkDebugUtilsMessengerEXT& GetDebugMessenger(); 
		const VkSurfaceKHR& GetSurface(); 
		const VkPhysicalDevice& GetPhysicalDevice();
		const VkPhysicalDeviceProperties& GetPhysicalProperties(); 
		const VkPhysicalDeviceFeatures& GetPhysicaFeatures();

		virtual void Init() override;
		virtual void ShutDown() override;  

	private: 
		void SetupInstance();
		void SetupDebugMessenger();  
		void SetupSurface(); 
		void SetupPhysicalDevice(); 
		void SetupLogicalDevice();

	private:
		bool CheckValidationLayerSupport();  
		int rateDeviceSuitability(VkPhysicalDevice device);
		std::vector<const char*> GetRequiredExtensions(); 
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		
	private:  
		VkDevice m_device;
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger; 
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		VkPhysicalDeviceProperties m_vkGPUProperties;
		VkPhysicalDeviceFeatures m_vkGPUFeatures; 

		QueueFamilyIndices m_queueFamilyIndices{};
		SwapChainSupportDetails m_swapChainSupportDetails{};

		const std::vector<const char*> m_DeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

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
