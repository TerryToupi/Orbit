#include "platform/Vulkan/GfxVulkanDevice.h"
#include "src/core/application.h"

#include <map> 
#include <set>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::string messageTypeAsString;

	switch (messageType)
	{
	case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
		messageTypeAsString = "[VULKAN_GENERAL]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
		messageTypeAsString = "[VULKAN_VALIDATION]";
 		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
		messageTypeAsString = "[VULKAN_PERFORMANCE]";
		break;
	case VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT:
		messageTypeAsString = "[VULKAN_DEVICE_ADDRESS_BINDING]";
		break;
	default:
		break;
	}

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		ENGINE_CORE_INFO("{} Validation layer: {}", messageTypeAsString, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		ENGINE_CORE_INFO("{} Validation layer: {}", messageTypeAsString, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		ENGINE_CORE_WARN("{} Validation layer: {}", messageTypeAsString, pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		ENGINE_CORE_ERROR("{} Validation layer: {}", messageTypeAsString, pCallbackData->pMessage);
		break;
	default:
		break;
	}

	return VK_FALSE;
}

namespace Engine
{
	void VulkanDevice::Init()
	{  
		ENGINE_CORE_INFO("Attempting to load Volk!");
		VK_VALIDATE(volkInitialize()); 

		SetupInstance(); 
		SetupDebugMessenger(); 
		SetupSurface();  
		SetupPhysicalDevice();
		SetupLogicalDevice();  

		m_properties.deviceName = std::string(m_vkGPUProperties.deviceName); 
		m_properties.vendorId = std::to_string(m_vkGPUProperties.vendorID); 
		m_properties.driverVersion = std::to_string(m_vkGPUProperties.driverVersion); 

		ENGINE_CORE_INFO("[VULKAN] Vulkan Device initialized!"); 
		ENGINE_CORE_INFO("[VULKAN] Device name: {}", m_properties.deviceName);
	}

	void VulkanDevice::ShutDown()
	{
		vkDeviceWaitIdle(m_device);

		vkDestroyDevice(m_device, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

		if (m_enableValidationLayers)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
			{
				func(m_instance, m_debugMessenger, nullptr);
			}
		}

		vkDestroyInstance(m_instance, nullptr);
	}

	void VulkanDevice::SetupInstance()
	{
		if (m_enableValidationLayers && !CheckValidationLayerSupport())
		{
			ENGINE_ASSERT(false, "Validation layers requested, but not available!");
		}

		// Application Info
		VkApplicationInfo appInfo =
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Orbit",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Orbit Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_3,
		};

		// Instance info
		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Get extensions.
		auto extensions = GetRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Enabling portability subset
		// if not for MoltenVK we wouldn't need this
		#ifdef OP_MACOS
		{
			createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		}
		#endif

		// Validation layers.
		if (m_enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = debugCallback,
			};
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		VK_VALIDATE(vkCreateInstance(&createInfo, nullptr, &m_instance)); 
		volkLoadInstance(m_instance);
	}

	void VulkanDevice::SetupDebugMessenger()
	{
		if (!m_enableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo =
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = debugCallback,
		};

		VkResult result;
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			result = func(m_instance, &createInfo, nullptr, &m_debugMessenger);
		}
		else
		{
			result = VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		VK_VALIDATE(result);
	}

	void VulkanDevice::SetupSurface()
	{
		VulkanWindow* wm = static_cast<VulkanWindow*>(Window::instance);

		VK_VALIDATE(glfwCreateWindowSurface(
			m_instance,
			wm->GetNativeWindow(),
			nullptr,
			&m_surface
		));
	}

	void VulkanDevice::SetupPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			ENGINE_ASSERT(false, "Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto& device : devices) {
			int score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		// Check if the best candidate is suitable at all
		if (candidates.rbegin()->first > 0) 
		{
			m_physicalDevice = candidates.rbegin()->second; 
			m_maxMSAAsamples = getMaxUsableSampleCount();
		}  
		else  
		{  
			ENGINE_ASSERT(false, "No GPU was suitable for usage!");
		}

		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_vkGPUProperties);
	} 

	VkSampleCountFlagBits VulkanDevice::getMaxUsableSampleCount() {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(m_physicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	int VulkanDevice::rateDeviceSuitability(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		int score = 0; 

		if (!IsDeviceSuitable(device))
			return score; 

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000; 

		score += deviceProperties.limits.maxImageDimension2D;
		
		return score;
	}

	void VulkanDevice::SetupLogicalDevice()
	{
		// Find swapchain capabilities
		m_swapChainSupportDetails = QuerySwapChainSupport(m_physicalDevice);

		// Find queue families
		m_queueFamilyIndices = FindQueueFamilies(m_physicalDevice);

		// Create queue families info
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { 
			m_queueFamilyIndices.graphicsFamily.value(), 
			m_queueFamilyIndices.presentFamily.value()
		};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamily,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority,
			};
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

		if (m_enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VK_VALIDATE(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device)); 
		volkLoadDevice(m_device);
	}

	// Helper

	bool VulkanDevice::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : m_validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> VulkanDevice::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		#ifdef OP_MACOS 
		{
			extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		}
		#endif 

		if (m_enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);   
		}

		if (!CheckInstanceExtensionSupport(extensions))
		{
			ENGINE_ASSERT(false, "Device does not support required extensions!")
		}

		return extensions;
	}

	bool VulkanDevice::CheckInstanceExtensionSupport(const std::vector<const char*>& glfwExtensionNames)
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		for (const char* extensionName : glfwExtensionNames)
		{
			bool extensionFound = false;

			for (const auto& extensionProperties : extensions)
			{
				if (strcmp(extensionName, extensionProperties.extensionName) == 0)
				{
					extensionFound = true; 
					ENGINE_CORE_TRACE("[VK INFO] Extension support: {}", extensionName);
					break;
				}
			}

			if (!extensionFound)
			{
				return false;
			}
		}

		return true;
	}

	QueueFamilyIndices VulkanDevice::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

	SwapChainSupportDetails VulkanDevice::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		// Formats set up.
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.Formats.data());
		}

		// Present mode set up.
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.PresentModes.data());
		}

		// Capabilities set up.
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.Capabilities);

		return details;
	}

	bool VulkanDevice::IsDeviceSuitable(VkPhysicalDevice device)
	{
		// Find queue families
		QueueFamilyIndices indices = FindQueueFamilies(device);

		// Check device extension support
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}
		bool extensionsSupported = requiredExtensions.empty();

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	const VkDevice& VulkanDevice::GetVkDevice()
	{ 
		return m_device;
	} 

	const VkInstance& VulkanDevice::GetVkInsance()
	{ 
		return m_instance;
	} 

	const VkDebugUtilsMessengerEXT& VulkanDevice::GetVkDebugMessenger()
	{ 
		return m_debugMessenger;
	} 

	const VkSurfaceKHR& VulkanDevice::GetVkSurface() 
	{ 
		return m_surface;
	} 

	const VkPhysicalDevice& VulkanDevice::GetVkPhysicalDevice()
	{ 
		return m_physicalDevice;
	} 

	const VkPhysicalDeviceProperties& VulkanDevice::GetVkPhysicalProperties()
	{ 
		return m_vkGPUProperties;
	} 

	const VkPhysicalDeviceFeatures& VulkanDevice::GetVkPhysicaFeatures()
	{ 
		return m_vkGPUFeatures;
	} 

	const QueueFamilyIndices& VulkanDevice::GetVkQueueFamilyIndices()
	{ 
		return m_queueFamilyIndices;
	} 

	const SwapChainSupportDetails& VulkanDevice::GetVkSwapChainSupportDetails()
	{ 
		return m_swapChainSupportDetails;
	} 

	const VkSampleCountFlagBits& VulkanDevice::GetMaxUsableSampleCount()
	{ 
		return m_maxMSAAsamples;
	}
}