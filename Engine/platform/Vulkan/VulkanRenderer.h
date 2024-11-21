#pragma once

#include "platform/Vulkan/VulkanCore.h" 
#include "platform/Vulkan/VulkanDevice.h" 
#include "platform/Vulkan/VulkanWindow.h"

#include "src/renderer/renderer.h"

namespace Engine
{
	class VulkanRenderer final : public Renderer
	{
	public: 
		virtual ~VulkanRenderer() = default; 

		virtual void Init() override;
		virtual void ShutDown() override;

		virtual void BeginFrame() override {};
		virtual void EndFrame() override {};
		virtual void Present() override {};

	private: 
		void createSwapChain();   
		void createImageViews();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities); 

	private:    
		// Queques setup data
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		// Swap chain data
		VkSwapchainKHR m_swapChain;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
	};
}