#include "platform/Vulkan/resources/VkTextures.h"  
#include "platform/Vulkan/VulkanDevice.h" 
#include "platform/Vulkan/VulkanRenderer.h"  
#include "platform/Vulkan/resources/VkResourceManager.h"

#include "platform/Vulkan/VulkanCore.h"

namespace Engine
{
	VkTexture::VkTexture(const TextureDesc&& desc)
	{ 
		VulkanDevice* device = (VulkanDevice*)Device::instance; 
		VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;
		VkResourceManager* rm = (VkResourceManager*)ResourceManager::instance;

		m_debugName = desc.debugName;
		m_extent = { desc.dimensions.x, desc.dimensions.y, desc.dimensions.z };

		VkImageUsageFlags usage = TextureUsageToVkVkImageUsageFlags(desc.usage); 
	
		usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		if (desc.usage != TextureUsage::DEPTH_STENCIL)
		{
			usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		} 

		VkImageCreateInfo imageInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = nullptr,
			.imageType = TextureTypeToVkImageType(desc.type),
			.format = TextureFormatToVkFormat(desc.format),
			.extent = m_extent,
			.mipLevels = desc.mipLevels,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = usage
		};

		VmaAllocationCreateInfo allocationInfo =
		{
			.usage = VMA_MEMORY_USAGE_GPU_ONLY,
			.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		VK_VALIDATE(vmaCreateImage(rm->GetVmaAllocator(), &imageInfo, &allocationInfo, &m_image, &m_allocation, nullptr));

		if (desc.data != nullptr)
		{
			VkBuffer stagingBuffer = VK_NULL_HANDLE; 
			VmaAllocation stagingAllocation = VK_NULL_HANDLE; 

			CreateStagingBuffer(&stagingBuffer, &stagingAllocation);

			VkDeviceSize imageSize = m_extent.width * m_extent.height * 4; 

			void* mappedData;
			vmaMapMemory(rm->GetVmaAllocator(), stagingAllocation, &mappedData);
			memcpy(mappedData, desc.data, (size_t)imageSize);
			vmaUnmapMemory(rm->GetVmaAllocator(), stagingAllocation);

			stbi_image_free(desc.data);

			CopyBufferToTexture(stagingBuffer);
		} 

		VkImageViewCreateInfo imageViewCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.image = m_image,
			.viewType = TextureTypeToVkVkImageViewType(desc.type),
			.format = TextureFormatToVkFormat(desc.internalFormat),
			.subresourceRange =
			{
				.aspectMask = TextureAspectToVkImageAspectFlags(desc.aspect),
				.baseMipLevel = 0, 
				.levelCount = desc.mipLevels,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		}; 

		VK_VALIDATE(vkCreateImageView(device->GetVkDevice(), &imageViewCreateInfo, nullptr, &m_imageView));
	} 

	void VkTexture::destroy()
	{ 
		VulkanDevice* device = (VulkanDevice*)Device::instance;
		VkResourceManager* rm = (VkResourceManager*)ResourceManager::instance; 

		vkDestroyImageView(device->GetVkDevice(), m_imageView, nullptr);
		vmaDestroyImage(rm->GetVmaAllocator(), m_image, m_allocation);
	}

	const char* VkTexture::GetDebugName()
	{
		return m_debugName;
	}

	VkImage& VkTexture::GetVkImage()
	{ 
		return m_image;
	}

	VkImageView& VkTexture::GetImageView()
	{ 
		return m_imageView;
	}

	VmaAllocation& VkTexture::GetAllocation()
	{ 
		return m_allocation;
	}

	VkExtent3D& VkTexture::GetExtent()
	{ 
		return m_extent;
	}

	void VkTexture::CreateStagingBuffer(VkBuffer* stagingBuffer, VmaAllocation* stagingAllocation)
	{ 
		VkResourceManager* rm = (VkResourceManager*)ResourceManager::instance; 

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

	void VkTexture::CopyBufferToTexture(VkBuffer stagingBuffer)
	{ 
		//VkImageSubresourceRange range =
		//{
		//	.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		//	.baseMipLevel = 0,
		//	.levelCount = 1,
		//	.baseArrayLayer = 0,
		//	.layerCount = 1,
		//};

		//VkImageMemoryBarrier imageBarrierToTransfer =
		//{
		//	.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		//	.srcAccessMask = 0,
		//	.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		//	.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		//	.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	.image = Image,
		//	.subresourceRange = range,
		//};

		//vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToTransfer);

		//VkBufferImageCopy copyRegion =
		//{
		//	.bufferOffset = 0,
		//	.bufferRowLength = 0,
		//	.bufferImageHeight = 0,
		//	.imageSubresource =
		//	{
		//		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		//		.mipLevel = 0,
		//		.baseArrayLayer = 0,
		//		.layerCount = 1,
		//	},
		//	.imageExtent = Extent,
		//};

		//vkCmdCopyBufferToImage(cmd, stagingBuffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		//VkImageMemoryBarrier imageBarrierToReadable =
		//{
		//	.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		//	.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		//	.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
		//	.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		//	.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		//	.image = Image,
		//	.subresourceRange = range,
		//};

		//vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrierToReadable);
	} 

	VkImageUsageFlags VkTexture::TextureUsageToVkVkImageUsageFlags(TextureUsage usage)
	{ 
		switch (usage)
		{
		case TextureUsage::COPY_SRC:
			return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		case TextureUsage::COPY_DST:
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::TEXTURE_BINDING:
			return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		case TextureUsage::STORAGE_BINDING:
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case TextureUsage::RENDER_ATTACHMENT:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case TextureUsage::SAMPLED:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case TextureUsage::DEPTH_STENCIL:
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		default:
			break;
		}

		return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
	}

	VkImageType VkTexture::TextureTypeToVkImageType(TextureType type)
	{
		switch (type)
		{
		case TextureType::D1:
			return VK_IMAGE_TYPE_1D;
		case TextureType::D2:
			return VK_IMAGE_TYPE_2D;
		case TextureType::D3:
			return VK_IMAGE_TYPE_3D;
		default:
			break;
		}

		return VK_IMAGE_TYPE_MAX_ENUM;
	}

	VkFormat VkTexture::TextureFormatToVkFormat(TextureFormat format)
	{ 
		switch (format)
		{
		case TextureFormat::RGB32_FLOAT: 
			return VK_FORMAT_R32G32B32_SFLOAT;
		case TextureFormat::D32_FLOAT: 
			return VK_FORMAT_D32_SFLOAT;
		case TextureFormat::RGBA16_FLOAT: 
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case TextureFormat::RGBA8_UNORM: 
			return VK_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RG16_FLOAT: 
			return VK_FORMAT_R16G16_SFLOAT;
		case TextureFormat::R32_FLOAT: 
			return VK_FORMAT_R32_SFLOAT;
		case TextureFormat::RGBA8_SRGB: 
			return VK_FORMAT_R8G8B8A8_SRGB; 
		case TextureFormat::BGRA8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case TextureFormat::BGRA8_SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
		} 

		return VK_FORMAT_MAX_ENUM;
	}

	VkImageAspectFlags VkTexture::TextureAspectToVkImageAspectFlags(TextureAspect aspect)
	{
		switch (aspect)
		{
		case TextureAspect::NONE:
			return VK_IMAGE_ASPECT_NONE;
		case TextureAspect::COLOR:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		case TextureAspect::DEPTH:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case TextureAspect::STENCIL:
			return VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
	} 

	VkImageViewType VkTexture::TextureTypeToVkVkImageViewType(TextureType textureType)
	{
		switch (textureType)
		{
		case TextureType::D1:
			return VK_IMAGE_VIEW_TYPE_1D;
		case TextureType::D2:
			return VK_IMAGE_VIEW_TYPE_2D;
		case TextureType::D3:
			return VK_IMAGE_VIEW_TYPE_3D;
		default:
			break;
		}

		return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	}
}