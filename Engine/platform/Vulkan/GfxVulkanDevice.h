#pragma once 
#include "src/renderer/device.h"

// Vulkan
#include "platform/Vulkan/GfxVulkanCore.h" 
#include "platform/Vulkan/GfxVulkanWindow.h"

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
		const VkDevice& GetVkDevice(); 
		const VkInstance& GetVkInsance(); 
		const VkDebugUtilsMessengerEXT& GetVkDebugMessenger(); 
		const VkSurfaceKHR& GetVkSurface(); 
		const VkPhysicalDevice& GetVkPhysicalDevice();
		const VkPhysicalDeviceProperties& GetVkPhysicalProperties(); 
		const VkPhysicalDeviceFeatures& GetVkPhysicaFeatures(); 
		const QueueFamilyIndices& GetVkQueueFamilyIndices();
		const SwapChainSupportDetails& GetVkSwapChainSupportDetails();

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
		VkSampleCountFlagBits getMaxUsableSampleCount();
		int rateDeviceSuitability(VkPhysicalDevice device); 
		bool CheckInstanceExtensionSupport(const std::vector<const char*>& glfwExtensionNames);
		std::vector<const char*> GetRequiredExtensions(); 
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device); 
		bool IsDeviceSuitable(VkPhysicalDevice device);
		
	private:  
		VkDevice m_device;
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger; 
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		VkPhysicalDeviceProperties m_vkGPUProperties;
		VkPhysicalDeviceFeatures m_vkGPUFeatures;  
		VkSampleCountFlagBits m_maxMSAAsamples;

		QueueFamilyIndices m_queueFamilyIndices{};
		SwapChainSupportDetails m_swapChainSupportDetails{};

		const std::vector<const char*> m_deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME, 

			#ifdef OP_MACOS
			// force portability subset for MoltenVK
			VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME 
			#endif
		};

		const std::vector<const char*> m_validationLayers =
		{
			"VK_LAYER_KHRONOS_validation"
		};  

		#ifdef DEBUG
		const bool m_enableValidationLayers = true;
		#elif PRODUCTION
		const bool m_enableValidationLayers = false;
		#endif
	};
}
