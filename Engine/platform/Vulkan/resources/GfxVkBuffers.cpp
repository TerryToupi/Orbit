#include "platform/Vulkan/resources/GfxVkBuffers.h"

#include "platform/Vulkan/GfxVulkanDevice.h" 
#include "platform/Vulkan/GfxVulkanRenderer.h"  
#include "platform/Vulkan/resources/GfxVkResourceManager.h"

#include "platform/Vulkan/GfxVulkanCore.h" 

#include "src/core/assert.h"

namespace Engine
{
	GfxVkBuffer::GfxVkBuffer(const BufferDesc&& desc)
	{ 
		VulkanDevice* device = (VulkanDevice*)Device::instance;
		VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;
		VkResourceManager* rm = (VkResourceManager*)ResourceManager::instance;

		m_debugName = desc.debugName; 
		m_byteSize = desc.size; 
		m_bufferUsageFlags = VkEnums::BufferUsageToVkBufferUsageFlags(desc.usage);
		m_memoryUsage = VkEnums::MemoryUsageToVmaMemoryUsage(desc.memoryUsage);

		VkBufferCreateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = m_byteSize,
			.usage = m_bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = 0,
			.pQueueFamilyIndices = nullptr,
		}; 

		VmaAllocationCreateInfo allocInfo =
		{
			.usage = m_memoryUsage,
		}; 

		VK_VALIDATE(vmaCreateBuffer(rm->GetVmaAllocator(), &info, &allocInfo, &m_buffer, &m_allocation, nullptr)); 

		if (desc.data != nullptr)
		{
			m_data = desc.data; 

			// Staging Buffer
			VkBuffer stagingBuffer = VK_NULL_HANDLE;
			VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE; 

			VkBufferCreateInfo info =
			{
				.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size = m_byteSize,
				.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr,
			};

			VmaAllocationCreateInfo allocInfo =
			{
				.usage = VMA_MEMORY_USAGE_CPU_ONLY,
			}; 

			VK_VALIDATE(vmaCreateBuffer(
				rm->GetVmaAllocator(),
				&info, 
				&allocInfo, 
				&stagingBuffer, 
				&stagingBufferAllocation, 
				nullptr
			));

			void* mappedData;
			vmaMapMemory(rm->GetVmaAllocator(), stagingBufferAllocation, &mappedData);
			memmove(mappedData, m_data, m_byteSize);
			vmaUnmapMemory(rm->GetVmaAllocator(), stagingBufferAllocation); 

			//renderer->ImmediateSubmit([=](VkCommandBuffer cmd)
			//	{
			//		VkBufferCopy copy =
			//		{
			//			.srcOffset = 0,
			//			.dstOffset = 0,
			//			.size = ByteSize,
			//		};

			//		vkCmdCopyBuffer(cmd, stagingBuffer, Buffer, 1, &copy);
			//	});

			vmaDestroyBuffer(rm->GetVmaAllocator(), stagingBuffer, stagingBufferAllocation);
		}
	} 

	void GfxVkBuffer::destroy()
	{ 
		VkResourceManager* rm = (VkResourceManager*)ResourceManager::instance;
		vmaDestroyBuffer(rm->GetVmaAllocator(), m_buffer, m_allocation);
	} 

	VkBuffer& GfxVkBuffer::GetVkBuffer()
	{ 
		return m_buffer;
	} 

	VmaAllocation& GfxVkBuffer::GetVmaAllocation()
	{ 
		return m_allocation;
	} 

	uint32_t& GfxVkBuffer::GetByteOffset()
	{ 
		return m_byteOffset;
	} 

	uint32_t& GfxVkBuffer::GetByteSize()
	{ 
		return m_byteSize;
	} 

	VkBufferUsageFlags& GfxVkBuffer::GetBufferUsageFlags()
	{ 
		return m_bufferUsageFlags;
	}  

	VmaMemoryUsage& GfxVkBuffer::GetMemoryUsage()
	{ 
		return m_memoryUsage;
	} 

	void* Engine::GfxVkBuffer::GetData()
	{
		return m_data;
	}
}