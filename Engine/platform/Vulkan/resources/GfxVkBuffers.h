#pragma once  

#include "src/renderer/resources/buffers.h" 
#include "platform/Vulkan/GfxVulkanCore.h"

namespace Engine
{
	class GfxVkBuffer
	{ 
	public:   
		GfxVkBuffer(const BufferDesc&& desc); 

		void destroy(); 

		VkBuffer& GetVkBuffer();
		VmaAllocation& GetVmaAllocation();
		uint32_t& GetByteOffset();
		uint32_t& GetByteSize();
		VkBufferUsageFlags& GetBufferUsageFlags();
		VmaMemoryUsage& GetMemoryUsage();
		void* GetData(); 

	private: 
		const char* m_debugName; 
		VkBuffer m_buffer = VK_NULL_HANDLE; 
		VmaAllocation m_allocation = VK_NULL_HANDLE; 
		uint32_t m_byteOffset = 0;
		uint32_t m_byteSize = 0; 
		VkBufferUsageFlags m_bufferUsageFlags = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
		VmaMemoryUsage m_memoryUsage = VMA_MEMORY_USAGE_MAX_ENUM;
		void* m_data = nullptr;
	};
}
