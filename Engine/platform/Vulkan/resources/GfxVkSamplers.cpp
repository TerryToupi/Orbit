#include "platform/Vulkan/resources/GfxVkSamplers.h"  
#include "platform/Vulkan/GfxVulkanDevice.h"

namespace Engine
{
	GfxVkSampler::GfxVkSampler(const SamplerDesc&& desc)
	{  
		VulkanDevice* device = (VulkanDevice*)Device::instance;

		VkSamplerCreateInfo info =
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.magFilter = VkEnums::FilterToVkFilter(desc.filters.mag),
			.minFilter = VkEnums::FilterToVkFilter(desc.filters.min),
			.mipmapMode = VkEnums::FilterToVkVkSamplerMipmapMode(desc.filters.mip),
			.addressModeU = VkEnums::WrapToVkSamplerAddressMode(desc.wrap),
			.addressModeV = VkEnums::WrapToVkSamplerAddressMode(desc.wrap),
			.addressModeW = VkEnums::WrapToVkSamplerAddressMode(desc.wrap),
			.compareEnable = desc.compare,
			.compareOp = desc.compare ? VkEnums::CompareToVkCompareOp(desc.compareType) : VK_COMPARE_OP_NEVER,
			.minLod = desc.minMip,
			.maxLod = desc.maxMip,
		}; 

		VK_VALIDATE(vkCreateSampler(device->GetVkDevice(), &info, nullptr, &m_sampler));
	} 

	void GfxVkSampler::destroy()
	{ 
		VulkanDevice* device = (VulkanDevice*)Device::instance;
		
		vkDestroySampler(device->GetVkDevice(), m_sampler, nullptr);
	} 

	const VkSampler& GfxVkSampler::GetVkSampler()
	{ 
		return m_sampler;
	}
}