#pragma once 

#include "src/renderer/resources/sampler.h"
#include "platform/Vulkan/GfxVulkanCore.h" 
#include "platform/Vulkan/resources/GfxVkEnums.h"

namespace Engine
{
	class GfxVkSampler
	{ 
	public:
		GfxVkSampler(const SamplerDesc&& desc);

		void destroy();   

		const VkSampler& GetVkSampler(); 

	private:  
		VkSampler m_sampler;
	}; 
}
