#pragma once 

#include "src/renderer/resources/textures.h"  
#include "platform/Vulkan/GfxVulkanCore.h" 
#include "platform/Vulkan/resources/GfxVkEnums.h"

namespace Engine
{
	class GfxVkTexture
	{ 
	public: 
		GfxVkTexture() = default;
		GfxVkTexture(const TextureDesc&& desc);
		
		void destroy();

		const char* GetDebugName();
		const VkImage& GetVkImage(); 
		const VkImageView& GetImageView();
		const VmaAllocation& GetAllocation();
		const VkExtent3D& GetExtent();

		void SetDebugName(const char* debugName);
		void SetVkImage(const VkImage& vkImage);
		void SetImageView(const VkImageView& imageView);
		void SetAllocation(const VmaAllocation& allocation);
		void SetExtent(const VkExtent3D& extent); 

	private: 
		void CreateStagingBuffer(VkBuffer* stagingBuffer, VmaAllocation* stagingAllocation);
		void CopyBufferToTexture(VkBuffer stagingBuffer, const TextureDesc& desc);

	private: 
		const char* m_debugName;  
		bool m_isSwapChainImage = false;
		VkImage m_image = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VkExtent3D m_extent;
	};
}