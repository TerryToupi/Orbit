#pragma once

#include "src/renderer/resources/renderPasses.h"  
#include "platform/Vulkan/resources/GfxVkEnums.h"
#include "platform/Vulkan/GfxVulkanCore.h"

namespace Engine
{
	class GfxVkRenderPass
	{ 
	public: 
		GfxVkRenderPass() = default; 
		GfxVkRenderPass(const RenderPassDesc&& desc); 

		void destroy(); 

		const char* GetDebugName();
		VkRenderPass& GetRenderPass();
		std::vector<bool>& GetColorClearValues();
		bool& GetDepthClearValue();

	private: 
		const char* m_debugName = nullptr; 
		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		std::vector<bool> m_colorClearValues;
		bool m_depthClearValue = false;
	};
}