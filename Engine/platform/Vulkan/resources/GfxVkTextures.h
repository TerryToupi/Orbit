#pragma once 

#include "src/renderer/resources/textures.h"  
#include "platform/Vulkan/GfxVulkanCore.h" 
#include "platform/Vulkan/resources/GfxVkEnums.h"

namespace Engine
{
	class GfxVkTexture
	{ 
	public:
		GfxVkTexture(const TextureDesc&& desc);
		
		void destroy();

		const char* GetDebugName();
		VkImage& GetVkImage(); 
		VkImageView& GetImageView();
		VmaAllocation& GetAllocation();
		VkExtent3D& GetExtent();

	private: 
		void CreateStagingBuffer(VkBuffer* stagingBuffer, VmaAllocation* stagingAllocation);
		void CopyBufferToTexture(VkBuffer stagingBuffer, const TextureDesc& desc);

	private: 
		const char* m_debugName; 
		VkImage m_image = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VkExtent3D m_extent;
	};
}