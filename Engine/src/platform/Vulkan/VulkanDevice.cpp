#include <platform/Vulkan/VulkanDevice.h>  
#include <core/application.h>

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
			ENGINE_CORE_ERROR("VALIDATION layers not available!");
			ENGINE_ASSERT(false);
		} 

		VkApplicationInfo appInfo =
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Orbit Application",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Orbit Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_2,
		}; 

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = GetRequiredExtensions(); 
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data(); 

		if (m_enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
			debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			debugCreateInfo.pfnUserCallback = debugCallback;
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

	int VulkanDevice::rateDeviceSuitability(VkPhysicalDevice device)
	{ 
		int score = 0; 

		QueueFamilyIndices indices = FindQueueFamilies(device);

		// Check device extension support
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());

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

		if (!indices.isComplete() && !extensionsSupported && !swapChainAdequate)
			return 0;

		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures); 

		// Discrete GPUs have a significant performance advantage
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		// Maximum possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		return score;
	}

	void VulkanDevice::SetupDebugMessenger()
	{
		if (!m_enableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;

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
		VK_VALIDATE(glfwCreateWindowSurface(
			m_instance,
			static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow()), 
			nullptr,
			&m_surface
			)
		);
	}

	void VulkanDevice::SetupPhysicalDevice()
	{ 
		std::multimap<int, VkPhysicalDevice> candidates; 

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		ENGINE_ASSERT(deviceCount);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()); 

		for (const auto& device : devices)
		{
			int score = rateDeviceSuitability(device); 
			candidates.insert(std::make_pair(score, device));
		} 

		if (candidates.rbegin()->first > 0)
		{
			m_physicalDevice = candidates.rbegin()->second;
		} 

		if (m_physicalDevice == VK_NULL_HANDLE)
		{
			ENGINE_CORE_ERROR("[VULKAN] Failed to setup a physical device!"); 
			ENGINE_ASSERT(false);
		} 

		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_vkGPUProperties);
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_vkGPUFeatures);
	}

	void VulkanDevice::SetupLogicalDevice()
	{
		m_swapChainSupportDetails = QuerySwapChainSupport(m_physicalDevice);
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
		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();

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

	std::vector<const char*> VulkanDevice::GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); 

		std::vector<const char*> requested_extensions(glfwExtensions, glfwExtensions + glfwExtensionCount); 

		if (m_enableValidationLayers)
		{
			requested_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		} 

		// Validate extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		ENGINE_ASSERT(extensionCount);

		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data()); 

		for (const char* extensionName : requested_extensions)
		{
			bool extensionFound = false; 

			for (const auto& extensionProperties : extensions)
			{
				if (strcmp(extensionName, extensionProperties.extensionName) == 0)
				{
					extensionFound = true;
					break;
				}
			}
			
			ENGINE_ASSERT(extensionFound);
			ENGINE_CORE_INFO("[VULKAN] Found extension: {}", std::string(extensionName));
		}

		return requested_extensions;
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

	const VkDevice& VulkanDevice::GetDevice()
	{ 
		return m_device;
	} 

	const VkInstance& VulkanDevice::GetInsance()
	{ 
		return m_instance;
	} 

	const VkDebugUtilsMessengerEXT& VulkanDevice::GetDebugMessenger()
	{ 
		return m_debugMessenger;
	} 

	const VkSurfaceKHR& VulkanDevice::GetSurface() 
	{ 
		return m_surface;
	} 

	const VkPhysicalDevice& VulkanDevice::GetPhysicalDevice()
	{ 
		return m_physicalDevice;
	} 

	const VkPhysicalDeviceProperties& VulkanDevice::GetPhysicalProperties()
	{ 
		return m_vkGPUProperties;
	} 

	const VkPhysicalDeviceFeatures& VulkanDevice::GetPhysicaFeatures()
	{ 
		return m_vkGPUFeatures;
	}
}