#include "platform/Vulkan/resources/VkResourceManager.h"  

#include "platform/Vulkan/VulkanDevice.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

namespace Engine
{
	void VkResourceManager::CreateMemoryAllocator()
	{   
		VulkanDevice* device = (VulkanDevice*)Device::instance;

		VmaVulkanFunctions vma_vulkan_func{}; 
		vma_vulkan_func.vkGetInstanceProcAddr = vkGetInstanceProcAddr; 
		vma_vulkan_func.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
		vma_vulkan_func.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
		vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
		vma_vulkan_func.vkAllocateMemory = vkAllocateMemory;
		vma_vulkan_func.vkBindBufferMemory = vkBindBufferMemory;
		vma_vulkan_func.vkBindImageMemory = vkBindImageMemory;
		vma_vulkan_func.vkCreateBuffer = vkCreateBuffer;
		vma_vulkan_func.vkCreateImage = vkCreateImage;
		vma_vulkan_func.vkDestroyBuffer = vkDestroyBuffer;
		vma_vulkan_func.vkDestroyImage = vkDestroyImage;
		vma_vulkan_func.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
		vma_vulkan_func.vkFreeMemory = vkFreeMemory;
		vma_vulkan_func.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
		vma_vulkan_func.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
		vma_vulkan_func.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
		vma_vulkan_func.vkMapMemory = vkMapMemory;
		vma_vulkan_func.vkUnmapMemory = vkUnmapMemory;
		vma_vulkan_func.vkCmdCopyBuffer = vkCmdCopyBuffer; 
		vma_vulkan_func.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
		vma_vulkan_func.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
		vma_vulkan_func.vkBindBufferMemory2KHR = vkBindBufferMemory2KHR;
		vma_vulkan_func.vkBindImageMemory2KHR = vkBindImageMemory2KHR;
		vma_vulkan_func.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
		vma_vulkan_func.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
		vma_vulkan_func.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
		
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.device = device->GetVkDevice();
		allocatorInfo.instance = device->GetVkInsance(); 
		allocatorInfo.physicalDevice = device->GetVkPhysicalDevice(); 
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		allocatorInfo.pVulkanFunctions = &vma_vulkan_func;

		VK_VALIDATE(vmaCreateAllocator(&allocatorInfo, &m_allocator));
	} 

	void VkResourceManager::Init()
	{ 
		CreateMemoryAllocator(); 
	} 

	void VkResourceManager::ShutDown()
	{ 
		vmaDestroyAllocator(m_allocator); 
	}
}