#include "platform/Vulkan/resources/GfxVkFrameBuffers.h"  
#include "platform/Vulkan/GfxVulkanDevice.h"
#include "platform/Vulkan/resources/GfxVkResourceManager.h"
#include "platform/Vulkan/resources/GfxVkTextures.h"

namespace Engine
{
	GfxVkFrameBuffer::GfxVkFrameBuffer(const FrameBufferDesc&& desc)
		:	m_debugName(desc.debugName), 
			m_frameBuffer(VK_NULL_HANDLE), 
			m_width(desc.width), 
			m_height(desc.height), 
			m_colorTargets(desc.colorTargets), 
			m_depthTarget(desc.depthTarget),
			m_renderPass(desc.renderPass)
	{ 
		validate();
	} 

	void GfxVkFrameBuffer::resize(uint32_t width, uint32_t height)
	{
		if (!m_resizable)
			return; 

		if (width == m_width && height == m_height)
			return; 

		m_width = width;
		m_height = height;

		validate();
	} 

	void GfxVkFrameBuffer::destroy()
	{ 
		VulkanDevice* device = (VulkanDevice*)Device::instance; 
		vkDestroyFramebuffer(device->GetVkDevice(), m_frameBuffer, nullptr);
	} 

	void GfxVkFrameBuffer::validate()
	{
		if (m_frameBuffer != VK_NULL_HANDLE)
			destroy();

		VulkanDevice* device = (VulkanDevice*)Device::instance;
		VulkanResourceManager* rm = (VulkanResourceManager*)ResourceManager::instance;

		std::vector<VkImageView> attachments; 
		attachments.reserve(m_colorTargets.size());

		for (const TEXTURE& colorTarget : m_colorTargets)
		{
			if (colorTarget.IsValid())
			{
				GfxVkTexture* colorTexture = rm->getTexture(colorTarget);  
				if (colorTexture != nullptr)
					attachments.push_back(colorTexture->GetImageView());
			}
		}

		if (m_depthTarget.IsValid())
		{
			GfxVkTexture* depthTexture = rm->getTexture(m_depthTarget); 
			if (depthTexture != nullptr)
				attachments.push_back(depthTexture->GetImageView());
		}

		ENGINE_ASSERT(m_renderPass.IsValid(), "Render pass was invalid");
		GfxVkRenderPass* vkRenderPass = rm->getRenderPass(m_renderPass);

		VkFramebufferCreateInfo frameBufferInfo =
		{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = nullptr,
			.renderPass = vkRenderPass->GetRenderPass(),
			.attachmentCount = (uint32_t)attachments.size(),
			.pAttachments = attachments.data(),
			.width = m_width,
			.height = m_height,
			.layers = 1,
		};

		VK_VALIDATE(vkCreateFramebuffer(device->GetVkDevice(), &frameBufferInfo, nullptr, &m_frameBuffer)); 
	} 

	const char* GfxVkFrameBuffer::GetDebugName()
	{
		return m_debugName;
	} 

	const VkFramebuffer& GfxVkFrameBuffer::GetFrameBuffer()
	{ 
		return m_frameBuffer;
	} 

	const uint32_t& GfxVkFrameBuffer::GetWidth()
	{ 
		return m_width;
	} 

	const uint32_t& GfxVkFrameBuffer::GetHeight()
	{ 
		return m_height;
	} 

	const std::vector<TEXTURE>& GfxVkFrameBuffer::GetColorTargets()
	{
		return m_colorTargets;
	} 

	const TEXTURE& GfxVkFrameBuffer::GetDepthTarget()
	{ 
		return m_depthTarget;
	} 

	const RENDERPASS& GfxVkFrameBuffer::GetRenderPass()
	{ 
		return m_renderPass;
	}
}