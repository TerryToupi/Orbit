#include "platform/Vulkan/VulkanRenderer.h"   

namespace Engine
{
    void VulkanRenderer::Init()
    { 
        VulkanDevice* device = (VulkanDevice*)Device::instance;

        vkGetDeviceQueue(device->GetVkDevice(), device->GetVkQueueFamilyIndices().graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(device->GetVkDevice(), device->GetVkQueueFamilyIndices().presentFamily.value(), 0, &m_presentQueue); 

        createSwapChain();
    } 

    void VulkanRenderer::ShutDown()
    { 
        VulkanDevice* device = (VulkanDevice*)Device::instance;  

        for (auto imageView : m_swapChainImageViews) {
            vkDestroyImageView(device->GetVkDevice(), imageView, nullptr);
        }

        vkDestroySwapchainKHR(device->GetVkDevice(), m_swapChain, nullptr);
    } 

    void VulkanRenderer::createSwapChain()
    { 
        VulkanDevice* device = (VulkanDevice*)Device::instance;

        SwapChainSupportDetails swapChainSupport = device->GetVkSwapChainSupportDetails();

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.Formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.PresentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.Capabilities);

        uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;

        if (swapChainSupport.Capabilities.maxImageCount > 0
            && imageCount > swapChainSupport.Capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.Capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = device->GetVkSurface();

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = device->GetVkQueueFamilyIndices();

        uint32_t queueFamilyIndices[] = { 
            indices.graphicsFamily.value(), 
            indices.presentFamily.value() 
        };

        if (indices.graphicsFamily != indices.presentFamily) 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        VK_VALIDATE(vkCreateSwapchainKHR(device->GetVkDevice(), &createInfo, nullptr, &m_swapChain));

        vkGetSwapchainImagesKHR(device->GetVkDevice(), m_swapChain, &imageCount, nullptr);
        m_swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device->GetVkDevice(), m_swapChain, &imageCount, m_swapChainImages.data());

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
    }

    void VulkanRenderer::createImageViews()
    {  
        VulkanDevice* device = (VulkanDevice*)Device::instance; 

        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImages.size(); i++) 
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapChainImages[i]; 

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapChainImageFormat; 

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; 

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VK_VALIDATE(vkCreateImageView(device->GetVkDevice(), &createInfo, nullptr, &m_swapChainImageViews[i]));
        }
    }

    VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
    {
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB 
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
    {
        for (const auto& availablePresentMode : availablePresentModes) 
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
    { 
        VulkanWindow* wm = (VulkanWindow*)Window::instance;

        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        {
            return capabilities.currentExtent;
        }
        else 
        {
            int width, height;
            glfwGetFramebufferSize(wm->GetNativeWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(
                actualExtent.width, capabilities.minImageExtent.width, 
                capabilities.maxImageExtent.width
            );
            actualExtent.height = std::clamp(
                actualExtent.height, capabilities.minImageExtent.height, 
                capabilities.maxImageExtent.height
            );

            return actualExtent;
        }
    }
}

