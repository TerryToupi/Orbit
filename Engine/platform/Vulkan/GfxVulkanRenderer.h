#pragma once

#include "platform/Vulkan/GfxVulkanCore.h" 
#include "platform/Vulkan/GfxVulkanDevice.h" 
#include "platform/Vulkan/resources/GfxVkEnums.h"
#include "platform/Vulkan/GfxVkCommandBuffers.h"

#include "src/renderer/renderer.h"  
#include "src/utilities/cleanupQueue.h"

namespace Engine
{  
	constexpr unsigned int FRAMES_IN_FLIGHT = 2;

	struct Frame
	{
		VkSemaphore ImageAvailableSemaphore; // Signal from swapchain. 
		VkSemaphore PresentRenderFinishedSemaphore;
		VkSemaphore GuiRenderFinishedSemaphore; // Signal that UI render is done.
		VkSemaphore BlitToSwapChainSemaphore;

		VkFence graphicsFence;

		VkCommandPool CommandPool;
		VkCommandBuffer MainCommandBuffer;
		VkCommandBuffer GuiCommandBuffer; 
		VkCommandBuffer BlitToSwapChainCommandBuffer;
	};

	class VulkanRenderer final : public Renderer
	{
	public: 
		virtual ~VulkanRenderer() = default; 

		virtual void Init() override;
		virtual void ShutDown() override;

		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void Present() override;  

		virtual void BlitToSwapChain(TEXTURE texture) override;

		virtual void OnResize(WindowResizeEvent& e) override; 

		CommandBuffer* BeginCommandRecording(const RenderPassStage stage, const CommandBufferType type) override;

		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& func); 

		void StubRenderPass();

		Frame& GetCurrentFrame();

		VkQueue& GetGraphicsQueue();
		VkQueue& GetPresentQueue(); 
		VkFormat& GetSwaipChainImageFormat();

	private: 
		void createSyncStructures();
		void createCommands();  
		void createDescriptorPool(); 

		void createSwapChain(); 
		void recreateSwapChain();
		void createSwapChainImageViews();
		void destroySwapChain();
		void createSwapChainFrameBuffers(); 
		void createBlitToSwapChainRenderPass();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); 

	private: 
		// enque functions for cleanup
		CleanUpQueue m_cleanup;

		// Queques setup data
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		// Swap chain data
		VkSwapchainKHR m_swapChain;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews; 
		std::vector<VkFramebuffer> m_swapChainFramebuffers; 
		RENDERPASS m_blitToSwapChainRenderpass; 
		RENDERPASSLAYOUT m_blitToSwapChainRenderpassLayout;

		//immediate submit data
		//TODO: replace this with a streaming commandBuffer running in parallel with frame
		struct ImmediateContext
		{
			VkFence UploadFence;
			VkCommandPool CommandPool;
			VkCommandBuffer CommandBuffer;
		} m_upload; 

		//Frame data for frames in flight
		Frame m_frameData[FRAMES_IN_FLIGHT];

		//Active Command buffers
		GfxVkCommandBuffer m_mainCommandBuffer[FRAMES_IN_FLIGHT];
		GfxVkCommandBuffer m_uiCommandBuffer[FRAMES_IN_FLIGHT];
		GfxVkCommandBuffer m_blitToSwapChainCommandBuffer[FRAMES_IN_FLIGHT];

		//Descriptor pools 
		VkDescriptorPool m_descriptorPool;

		//Frame indexes
		uint64_t m_frameIndex = 0;

		//Should recreate swapchain
		bool m_framebufferResized = false; 
		uint32_t m_swapChainImageIndex = 0;
	};
}