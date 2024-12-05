#include "platform/Vulkan/resources/GfxVkRenderPasses.h" 
#include "platform/Vulkan/GfxVulkanRenderer.h"
#include "platform/Vulkan/GfxVulkanDevice.h"
#include "platform/Vulkan/resources/GfxVkResourceManager.h"
#include "platform/Vulkan/resources/GfxVkEnums.h"

namespace Engine
{
	GfxVkRenderPass::GfxVkRenderPass(const RenderPassDesc&& desc)
	{ 
		VulkanDevice* device = (VulkanDevice*)Device::instance;
		VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;
		VkResourceManager* rm = (VkResourceManager*)ResourceManager::instance;

		m_debugName = desc.debugName;

		GfxVkRenderPassLayout* layout = rm->getRenderPassLayout(desc.layout);

		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> colorAttachmentRefs;

		uint32_t index = 0;

		for (const auto& colorTarget : desc.colorTargets)
		{
			m_colorClearValues.push_back(colorTarget.loadOp == LoadOperation::CLEAR);
			
			VkFormat format;
			if (colorTarget.format == TextureFormat::BACKBUFFER)
				format = renderer->GetSwaipChainImageFormat();
			else
				format = VkEnums::TextureFormatToVkFormat(colorTarget.format);
			
			attachments.push_back(VkAttachmentDescription
				{
					.format = format,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VkEnums::LoadOperationToVkAttachmentLoadOp(colorTarget.loadOp),
					.storeOp = VkEnums::StoreOperationVkAttachmentStoreOp(colorTarget.storeOp),
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VkEnums::TextureLayoutToVkImageLayout(colorTarget.prevUsage),
					.finalLayout = VkEnums::TextureLayoutToVkImageLayout(colorTarget.nextUsage),
				});

			colorAttachmentRefs.push_back(VkAttachmentReference
				{
					.attachment = index++,
					.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				});
		}

		VkAttachmentReference depthAttachmentRef;

		for (const auto& subpass : layout->getSubPasses())
		{
			if (subpass.depthTarget)
			{
				m_depthClearValue = (desc.depthTarget.loadOp == LoadOperation::CLEAR);

				attachments.push_back(VkAttachmentDescription
					{
						.flags = 0,
						.format = VkEnums::TextureFormatToVkFormat(layout->getDepthStencilFormat()),
						.samples = VK_SAMPLE_COUNT_1_BIT,
						.loadOp = VkEnums::LoadOperationToVkAttachmentLoadOp(desc.depthTarget.loadOp),
						.storeOp = VkEnums::StoreOperationVkAttachmentStoreOp(desc.depthTarget.storeOp),
						.stencilLoadOp = VkEnums::LoadOperationToVkAttachmentLoadOp(desc.depthTarget.stencilLoadOp),
						.stencilStoreOp = VkEnums::StoreOperationVkAttachmentStoreOp(desc.depthTarget.stencilStoreOp),
						.initialLayout = VkEnums::TextureLayoutToVkImageLayout(desc.depthTarget.prevUsage),
						.finalLayout = VkEnums::TextureLayoutToVkImageLayout(desc.depthTarget.nextUsage),
					});

				depthAttachmentRef =
				{
					.attachment = index++,
					.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				};
			}
		}

		//TODO: refuctor to accept multiple subpasses
		VkSubpassDescription subpass =
		{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = colorAttachmentRefs.data(),
			.pDepthStencilAttachment = &depthAttachmentRef,
		};

		VkSubpassDependency dependency =
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		};

		VkSubpassDependency depthDependency =
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		};

		VkSubpassDependency dependencies[2] = { dependency, depthDependency };

		VkRenderPassCreateInfo renderPassInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 2,
			.pAttachments = attachments.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 2,
			.pDependencies = &dependencies[0],
		};

		VK_VALIDATE(vkCreateRenderPass(device->GetVkDevice(), &renderPassInfo, nullptr, &m_renderPass));
	} 

	void GfxVkRenderPass::destroy()
	{ 
		VulkanDevice* device = (VulkanDevice*)Device::instance;
		vkDestroyRenderPass(device->GetVkDevice(), m_renderPass, nullptr);
	} 

	const char* GfxVkRenderPass::GetDebugName()
	{
		return m_debugName;
	} 

	VkRenderPass& GfxVkRenderPass::GetRenderPass()
	{ 
		return m_renderPass;
	} 

	std::vector<bool>& GfxVkRenderPass::GetColorClearValues()
	{ 
		return m_colorClearValues;
	} 

	bool& GfxVkRenderPass::GetDepthClearValue()
	{ 
		return m_depthClearValue;
	}
}