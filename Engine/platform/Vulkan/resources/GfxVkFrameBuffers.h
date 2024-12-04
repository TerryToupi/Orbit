#pragma once

#include "src/utilities/handles.h"   
#include "src/renderer/resources/frameBuffers.h"   
#include "src/renderer/resources/textures.h"   
#include "src/renderer/resources/renderPasses.h"   
#include "platform/Vulkan/GfxVulkanCore.h" 

#include <initializer_list>

namespace Engine
{
	class GfxVkFrameBuffer
	{  
	public:
		GfxVkFrameBuffer() = default; 
		GfxVkFrameBuffer(const FrameBufferDesc&& desc);

		void resize(uint32_t width, uint32_t height);  
		void destroy();
		void validate();
	
		const char* GetDebugName();
		VkFramebuffer& GetFrameBuffer();
		uint32_t& GetWidth();
		uint32_t& GetHeight();
		std::initializer_list<Handle<Texture>>& GetColorTargets();
		Handle<Texture>& GetDepthTarget();
		Handle<RenderPass>& GetRenderPass();

	private: 
		const char* m_debugName = "";
		VkFramebuffer m_frameBuffer = VK_NULL_HANDLE;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		std::initializer_list<Handle<Texture>> m_colorTargets;
		Handle<Texture> m_depthTarget;
		Handle<RenderPass> m_renderPass;
	};
}