#pragma once 

#include "src/renderer/resources/textures.h"  
#include "platform/Vulkan/VulkanCore.h"

namespace Engine
{
	class VkTexture
	{ 
	public:
		VkTexture(const TextureDesc&& desc);
		
		void destroy();

		const char* GetDebugName();
		VkImage& GetVkImage(); 
		VkImageView& GetImageView();
		VmaAllocation& GetAllocation();
		VkExtent3D& GetExtent();

		static VkImageUsageFlags TextureUsageToVkVkImageUsageFlags(TextureUsage usage); 
		static VkImageType TextureTypeToVkImageType(TextureType type); 
		static VkFormat TextureFormatToVkFormat(TextureFormat format);
		static VkImageAspectFlags TextureAspectToVkImageAspectFlags(TextureAspect aspect);
		static VkImageViewType TextureTypeToVkVkImageViewType(TextureType textureType);

	private: 
		void CreateStagingBuffer(VkBuffer* stagingBuffer, VmaAllocation* stagingAllocation);
		void CopyBufferToTexture(VkBuffer stagingBuffer);

	private: 
		const char* m_debugName; 
		VkImage m_image = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VkExtent3D m_extent;
	};
}