#include "platform/Vulkan/resources/GfxVkResourceManager.h"  
#include "platform/Vulkan/GfxVulkanDevice.h" 

#include <functional>

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

	SHADER VkResourceManager::createShader(const ShaderDesc&& desc)
	{
		return m_shaders.Insert(GfxVkShader(std::forward<const ShaderDesc>(desc)));
	}

	BINDGROUP VkResourceManager::createBindgroup(const BindGroupDesc&& desc)
	{
		return m_bindGroups.Insert(GfxVkBindGroup(std::forward<const BindGroupDesc>(desc)));
	}

	BINDGROUPLAYOUT VkResourceManager::createBindgroupLayout(const BindGroupLayoutDesc&& desc)
	{
		return m_bindGroupLayouts.Insert(GfxVkBindGroupLayout(std::forward<const BindGroupLayoutDesc>(desc)));
	}

	TEXTURE VkResourceManager::createTexture(const TextureDesc&& desc)
	{
		return m_textures.Insert(GfxVkTexture(std::forward<const TextureDesc>(desc)));
	}

	SAMPLER VkResourceManager::createSampler(const SamplerDesc&& desc)
	{
		return m_samplers.Insert(GfxVkSampler(std::forward<const SamplerDesc>(desc)));
	}

	BUFFER VkResourceManager::createBuffer(const BufferDesc&& desc)
	{
		return m_buffers.Insert(GfxVkBuffer(std::forward<const BufferDesc>(desc)));
	}

	MESH VkResourceManager::createMesh(const MeshDesc&& desc)
	{
		return m_meshes.Insert(GfxVkMesh(std::forward<const MeshDesc>(desc)));
	}

	RENDERPASSLAYOUT VkResourceManager::createRenderPassLayout(const RenderPassLayoutDesc&& desc)
	{
		return m_renderPassLayouts.Insert(GfxVkRenderPassLayout(std::forward<const RenderPassLayoutDesc>(desc)));
	}

	RENDERPASS VkResourceManager::createRenderPass(const RenderPassDesc&& desc)
	{
		return m_renderPasses.Insert(GfxVkRenderPass(std::forward<const RenderPassDesc>(desc)));
	}

	FRAMEBUFFER VkResourceManager::createFrameBuffer(const FrameBufferDesc&& desc)
	{
		return m_frameBuffers.Insert(GfxVkFrameBuffer(std::forward<const FrameBufferDesc>(desc)));
	} 

	void VkResourceManager::destroyShader(SHADER handle)
	{
		GfxVkShader* shader = m_shaders.Get(handle);  
		if (shader)
			shader->destroy();
		m_shaders.Remove(handle);
	} 

	void VkResourceManager::destroyBindgroup(BINDGROUP handle)
	{
		GfxVkBindGroup* bg = m_bindGroups.Get(handle);
		if (bg)
			bg->destroy(); 
		m_bindGroups.Remove(handle);
	} 

	void VkResourceManager::destroyBindgroupLayout(BINDGROUPLAYOUT handle)
	{
		GfxVkBindGroupLayout* bgl = m_bindGroupLayouts.Get(handle);
		if (bgl)
			bgl->destroy(); 
		m_bindGroupLayouts.Remove(handle);
	}

	void VkResourceManager::destroyTexture(TEXTURE handle)
	{
		GfxVkTexture* texture = m_textures.Get(handle); 
		if (texture)
			texture->destroy(); 
		m_textures.Remove(handle);
	} 

	void VkResourceManager::destroySampler(SAMPLER handle)
	{ 
		GfxVkSampler* sampler = m_samplers.Get(handle); 
		if (sampler)
			sampler->destroy(); 
		m_samplers.Remove(handle);
	} 

	void VkResourceManager::destroyBuffer(BUFFER handle)
	{ 
		GfxVkBuffer* buffer = m_buffers.Get(handle); 
		if (buffer)
			buffer->destroy(); 
		m_buffers.Remove(handle);
	} 

	void VkResourceManager::destroyMesh(MESH handle)
	{
		GfxVkMesh* mesh = m_meshes.Get(handle); 
		if (mesh)
			mesh->destroy(); 
		m_meshes.Remove(handle);
	} 

	void VkResourceManager::destroyRenderPassLayout(RENDERPASSLAYOUT handle)
	{
		GfxVkRenderPassLayout* rpl = m_renderPassLayouts.Get(handle); 
		if (rpl)
			rpl->destroy();
		m_renderPassLayouts.Remove(handle);
	} 

	void VkResourceManager::destroyRenderPass(RENDERPASS handle)
	{
		GfxVkRenderPass* rp = m_renderPasses.Get(handle); 
		if (rp)
			rp->destroy(); 
		m_renderPasses.Remove(handle);
	} 

	void VkResourceManager::destroyFrameBuffer(FRAMEBUFFER handle)
	{
		GfxVkFrameBuffer* fb = m_frameBuffers.Get(handle); 
		if (fb)
			fb->destroy(); 
		m_frameBuffers.Remove(handle);
	} 

	GfxVkShader* VkResourceManager::getShader(SHADER handle)
	{
		return m_shaders.Get(handle);
	} 

	GfxVkBindGroup* VkResourceManager::getBindgroup(BINDGROUP handle)
	{
		return m_bindGroups.Get(handle);
	} 

	GfxVkBindGroupLayout* VkResourceManager::getBindgroupLayout(BINDGROUPLAYOUT handle)
	{
		return m_bindGroupLayouts.Get(handle);
	} 

	GfxVkTexture* VkResourceManager::getTexture(TEXTURE handle)
	{
		return m_textures.Get(handle);
	} 

	GfxVkSampler* VkResourceManager::getSampler(SAMPLER handle)
	{
		return m_samplers.Get(handle);
	} 

	GfxVkBuffer* VkResourceManager::getBuffer(BUFFER handle)
	{
		return m_buffers.Get(handle);
	} 

	GfxVkMesh* VkResourceManager::getMesh(MESH handle)
	{
		return m_meshes.Get(handle);
	} 

	GfxVkRenderPassLayout* VkResourceManager::getRenderPassLayout(RENDERPASSLAYOUT handle)
	{
		return m_renderPassLayouts.Get(handle);
	} 

	GfxVkRenderPass* VkResourceManager::getRenderPass(RENDERPASS handle)
	{
		return m_renderPasses.Get(handle);
	} 

	GfxVkFrameBuffer* VkResourceManager::getFrameBuffer(FRAMEBUFFER handle)
	{
		return m_frameBuffers.Get(handle);
	} 

	SHADER VkResourceManager::appendShader(const GfxVkShader& shader)
	{
		return m_shaders.Insert(shader);
	} 

	BINDGROUP VkResourceManager::appendBindgroup(const GfxVkBindGroup& bindGroup)
	{
		return m_bindGroups.Insert(bindGroup);
	} 

	BINDGROUPLAYOUT VkResourceManager::appendBindgroupLayout(const GfxVkBindGroupLayout& bindGroupLayout)
	{
		return m_bindGroupLayouts.Insert(bindGroupLayout);
	} 

	TEXTURE VkResourceManager::appendTexture(const GfxVkTexture& texture)
	{
		return m_textures.Insert(texture);
	} 

	SAMPLER VkResourceManager::appendSampler(const GfxVkSampler& sampler)
	{
		return m_samplers.Insert(sampler);
	} 

	BUFFER VkResourceManager::appendBuffer(const GfxVkBuffer& buffer)
	{
		return m_buffers.Insert(buffer);
	} 

	MESH VkResourceManager::appendMesh(const GfxVkMesh& mesh)
	{
		return m_meshes.Insert(mesh);
	} 

	RENDERPASSLAYOUT VkResourceManager::appendRenderPassLayout(const GfxVkRenderPassLayout& renderPassLayout)
	{
		return m_renderPassLayouts.Insert(renderPassLayout);
	} 

	RENDERPASS VkResourceManager::appendRenderPass(const GfxVkRenderPass& renderPass)
	{
		return m_renderPasses.Insert(renderPass);
	} 

	FRAMEBUFFER VkResourceManager::appendFrameBuffer(const GfxVkFrameBuffer& frameBuffer)
	{
		return m_frameBuffers.Insert(frameBuffer);
	}
}
