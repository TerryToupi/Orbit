#include "platform/Vulkan/resources/GfxVkTextures.h"  
#include "platform/Vulkan/GfxVulkanDevice.h" 
#include "platform/Vulkan/GfxVulkanRenderer.h"  
#include "platform/Vulkan/resources/GfxVkResourceManager.h"

#include "platform/Vulkan/GfxVulkanCore.h" 

#include "src/core/assert.h"

namespace Engine
{
	GfxVkTexture::GfxVkTexture(const TextureDesc&& desc)
	{ 
		VulkanDevice* device = (VulkanDevice*)Device::instance; 
		VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;
		VulkanResourceManager* rm = (VulkanResourceManager*)ResourceManager::instance;

		m_debugName = desc.debugName;
		m_extent = { desc.dimensions.x, desc.dimensions.y, desc.dimensions.z }; 

		ENGINE_CORE_TRACE(m_debugName);

		VkImageUsageFlags usage = VkEnums::TextureUsageToVkVkImageUsageFlags(desc.usage); 
		
		if (desc.data != nullptr)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		if (desc.usage != TextureUsage::DEPTH_STENCIL)
		{
			usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		} 

		VkImageCreateInfo imageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.imageType = VkEnums::TextureTypeToVkImageType(desc.type),
			.format = VkEnums::TextureFormatToVkFormat(desc.format),
			.extent = m_extent,
			.mipLevels = desc.mipLevels, 
			.arrayLayers = desc.arrayLayers,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = usage,
		};

		VmaAllocationCreateInfo allocationInfo =
		{
			.usage = VMA_MEMORY_USAGE_GPU_ONLY,
			.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		VK_VALIDATE(vmaCreateImage(
			rm->GetVmaAllocator(), 
			&imageInfo, 
			&allocationInfo, 
			&m_image, 
			&m_allocation, 
			nullptr
		));

		if (desc.data != nullptr)
		{
			VkBuffer stagingBuffer = VK_NULL_HANDLE; 
			VmaAllocation stagingAllocation = VK_NULL_HANDLE; 

			CreateStagingBuffer(&stagingBuffer, &stagingAllocation);

			VkDeviceSize imageSize = m_extent.width * m_extent.height * 4; 

			void* mappedData;
			vmaMapMemory(rm->GetVmaAllocator(), stagingAllocation, &mappedData);
			memmove(mappedData, desc.data, (size_t)imageSize);
			vmaUnmapMemory(rm->GetVmaAllocator(), stagingAllocation);

			stbi_image_free(desc.data);

			CopyBufferToTexture(stagingBuffer, desc);
		} 

		VkImageViewCreateInfo imageViewCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.image = m_image,
			.viewType =VkEnums::TextureTypeToVkVkImageViewType(desc.type),
			.format = VkEnums::TextureFormatToVkFormat(desc.internalFormat),
			.subresourceRange =
			{
				.aspectMask = VkEnums::TextureAspectToVkImageAspectFlags(desc.aspect),
				.baseMipLevel = 0, 
				.levelCount = desc.mipLevels,
				.baseArrayLayer = 0,
				.layerCount = desc.arrayLayers,
			}
		}; 

		VK_VALIDATE(vkCreateImageView(device->GetVkDevice(), &imageViewCreateInfo, nullptr, &m_imageView));
	} 

	void GfxVkTexture::destroy()
	{ 
		VulkanDevice* device = (VulkanDevice*)Device::instance;
		VulkanResourceManager* rm = (VulkanResourceManager*)ResourceManager::instance; 
		
		ENGINE_CORE_INFO("Desrtoying, {}", m_debugName);
		
		vkDestroyImageView(device->GetVkDevice(), m_imageView, nullptr);
		if (m_allocation)
			vmaDestroyImage(rm->GetVmaAllocator(), m_image, m_allocation);
	}

	const char* GfxVkTexture::GetDebugName()
	{
		return m_debugName;
	}

	const VkImage& GfxVkTexture::GetVkImage()
	{ 
		return m_image;
	}

	const VkImageView& GfxVkTexture::GetImageView()
	{ 
		return m_imageView;
	}

	const VmaAllocation& GfxVkTexture::GetAllocation()
	{ 
		return m_allocation;
	}

	const VkExtent3D& GfxVkTexture::GetExtent()
	{ 
		return m_extent;
	}

	void GfxVkTexture::SetDebugName(const char* debugName)
	{ 
		m_debugName = debugName;
	}

	void GfxVkTexture::SetVkImage(const VkImage& vkImage)
	{ 
		m_image = vkImage;
	}

	void GfxVkTexture::SetImageView(const VkImageView& imageView)
	{ 
		m_imageView = imageView;
	}

	void GfxVkTexture::SetAllocation(const VmaAllocation& allocation)
	{ 
		m_allocation = allocation;
	}

	void GfxVkTexture::SetExtent(const VkExtent3D& extent)
	{ 
		m_extent = extent;
	}

	void GfxVkTexture::CreateStagingBuffer(VkBuffer* stagingBuffer, VmaAllocation* stagingAllocation)
	{ 
		VulkanResourceManager* rm = (VulkanResourceManager*)ResourceManager::instance; 

		VkDeviceSize imageSize = m_extent.width * m_extent.height * 4;

		VkBufferCreateInfo stagingBufferCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = imageSize,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
		};

		VmaAllocationCreateInfo vmaStagingAllocCreateInfo =
		{
			.usage = VMA_MEMORY_USAGE_CPU_ONLY,
		};

		VK_VALIDATE(vmaCreateBuffer(
			rm->GetVmaAllocator(),
			&stagingBufferCreateInfo,
			&vmaStagingAllocCreateInfo,
			stagingBuffer,
			stagingAllocation,
			nullptr
		));
	} 

	void GfxVkTexture::CopyBufferToTexture(VkBuffer stagingBuffer, const TextureDesc& desc)
	{
		VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;

		renderer->ImmediateSubmit([&](VkCommandBuffer cmd)
		{
			VkImageSubresourceRange range =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = desc.mipLevels,
				.baseArrayLayer = 0,
				.layerCount = desc.arrayLayers,
			};

			VkImageMemoryBarrier imageBarrierToTransfer =
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.image = m_image,
				.subresourceRange = range,
			};

			vkCmdPipelineBarrier(
				cmd,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0,
				nullptr,
				0,
				nullptr,
				1,
				&imageBarrierToTransfer
			);

			VkBufferImageCopy copyRegion =
			{
				.bufferOffset = 0,
				.bufferRowLength = 0,
				.bufferImageHeight = 0,
				.imageSubresource =
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.mipLevel = desc.mipLevels,
					.baseArrayLayer = 0,
					.layerCount = desc.arrayLayers,
				},
				.imageExtent = m_extent,
			};

			vkCmdCopyBufferToImage(
				cmd, 
				stagingBuffer, 
				m_image, 
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				1, 
				&copyRegion
			);

			VkImageMemoryBarrier imageBarrierToReadable =
			{
				.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
				.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.image = m_image,
				.subresourceRange = range,
			};

			vkCmdPipelineBarrier(
				cmd, 
				VK_PIPELINE_STAGE_TRANSFER_BIT, 
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
				0, 
				0, 
				nullptr, 
				0, 
				nullptr, 
				1, 
				&imageBarrierToReadable
			);
		});
	}
}