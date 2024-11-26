#pragma once 

#include "platform/Vulkan/GfxVulkanCore.h" 
#include "src/renderer/resources/enums.h"

namespace Engine
{
	namespace VkEnums
	{
		VkImageUsageFlags TextureUsageToVkVkImageUsageFlags(TextureUsage usage);
		VkImageType TextureTypeToVkImageType(TextureType type);
		VkFormat TextureFormatToVkFormat(TextureFormat format);
		VkImageAspectFlags TextureAspectToVkImageAspectFlags(TextureAspect aspect);
		VkImageViewType TextureTypeToVkVkImageViewType(TextureType textureType); 
		VkFilter FilterToVkFilter(Filter filter);
		VkSamplerMipmapMode FilterToVkVkSamplerMipmapMode(Filter filter);
		VkSamplerAddressMode WrapToVkSamplerAddressMode(Wrap wrap); 
		VkCompareOp CompareToVkCompareOp(Compare compare);
		VkBufferUsageFlags BufferUsageToVkBufferUsageFlags(BufferUsage bufferUsage); 
		VmaMemoryUsage MemoryUsageToVmaMemoryUsage(MemoryUsage memoryUsage);
	}
}