#include "platform/Vulkan/GfxVulkanRenderer.h"   

namespace Engine
{
    void VulkanRenderer::Init()
    { 
        VulkanDevice* device = (VulkanDevice*)Device::instance;

        vkGetDeviceQueue(device->GetVkDevice(), device->GetVkQueueFamilyIndices().graphicsFamily.value(), 0, &m_graphicsQueue);
        vkGetDeviceQueue(device->GetVkDevice(), device->GetVkQueueFamilyIndices().presentFamily.value(), 0, &m_presentQueue); 

        createSwapChain();
        createImageViews(); 
        createCommands();
        createSyncStructures(); 
        createDescriptorPool();

        ENGINE_CORE_INFO("[VULKAN] Vulkan Renderer initialized!");
    }

    void VulkanRenderer::ShutDown()
    {  
        VulkanDevice* device = (VulkanDevice*)Device::instance; 

        vkDeviceWaitIdle(device->GetVkDevice()); 

        m_cleanup.Flush();
    } 

    void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& func)
    {
        VulkanDevice* device = (VulkanDevice*)Device::instance;

        VkCommandBuffer cmd = m_upload.CommandBuffer;

        VkCommandBufferBeginInfo cmdBeginInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr,
        };

        VK_VALIDATE(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        func(cmd);

        VK_VALIDATE(vkEndCommandBuffer(cmd));

        VkSubmitInfo submit =
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
        };

        // Submit command buffer to the queue and execute it.
        // UploadFence will now block until the graphic commands finish execution
        VK_VALIDATE(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_upload.UploadFence));

        vkWaitForFences(device->GetVkDevice(), 1, &m_upload.UploadFence, true, 9999999999);
        vkResetFences(device->GetVkDevice(), 1, &m_upload.UploadFence);

        // Reset the command buffers inside the command pool
        vkResetCommandPool(device->GetVkDevice(), m_upload.CommandPool, 0);
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
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
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

        m_cleanup.appendFunction([=]()
            {
                vkDestroySwapchainKHR(device->GetVkDevice(), m_swapChain, nullptr);
            });
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

        m_cleanup.appendFunction([=]()
            {
                for (auto imageView : m_swapChainImageViews) {
                    vkDestroyImageView(device->GetVkDevice(), imageView, nullptr);
                }
            });
    }

    void VulkanRenderer::createSyncStructures()
    { 
        VulkanDevice* device = (VulkanDevice*)Device::instance;

        // Create synchronization structures.
        VkFenceCreateInfo fenceCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };


        // For the semaphores we don't need any flags.
        VkSemaphoreCreateInfo semaphoreCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VK_VALIDATE(vkCreateFence(device->GetVkDevice(), &fenceCreateInfo, nullptr, &m_frameData[i].flightFence));

            //enqueue the destruction of the fence
            m_cleanup.appendFunction([=]()
                {
                    vkDestroyFence(device->GetVkDevice(), m_frameData[i].flightFence, nullptr);
                });

            VK_VALIDATE(vkCreateSemaphore(device->GetVkDevice(), &semaphoreCreateInfo, nullptr, &m_frameData[i].ImageAvailableSemaphore));
            VK_VALIDATE(vkCreateSemaphore(device->GetVkDevice(), &semaphoreCreateInfo, nullptr, &m_frameData[i].PresentRenderFinishedSemaphore));
            VK_VALIDATE(vkCreateSemaphore(device->GetVkDevice(), &semaphoreCreateInfo, nullptr, &m_frameData[i].GuiRenderFinishedSemaphore));

            //enqueue the destruction of semaphores
            m_cleanup.appendFunction([=]()
                {
                    vkDestroySemaphore(device->GetVkDevice(), m_frameData[i].ImageAvailableSemaphore, nullptr);
                    vkDestroySemaphore(device->GetVkDevice(), m_frameData[i].PresentRenderFinishedSemaphore, nullptr);
                    vkDestroySemaphore(device->GetVkDevice(), m_frameData[i].GuiRenderFinishedSemaphore, nullptr);
                }
            );
        }

        VkFenceCreateInfo uploadFenceCreateInfo =
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
        };

        VK_VALIDATE(vkCreateFence(
            device->GetVkDevice(),
            &uploadFenceCreateInfo,
            nullptr, 
            &m_upload.UploadFence
        ));

        m_cleanup.appendFunction([=]()
			{
				vkDestroyFence(device->GetVkDevice(), m_upload.UploadFence, nullptr);
			}
        );
    }

    void VulkanRenderer::createCommands() {

        VulkanDevice* device = (VulkanDevice*)Device::instance;

        const QueueFamilyIndices& indices = device->GetVkQueueFamilyIndices();

        // Create a command pool for commands submitted to the graphics queue. We also want the pool to allow for resetting of individual command buffers
        VkCommandPoolCreateInfo commandPoolInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = indices.graphicsFamily.value(),
        };

        // VkCommandPoolCreateInfo secondaryCommandPoolInfo =
        // {
        //     .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        //     .pNext = nullptr,
        //     .flags = 0,
        //     .queueFamilyIndex = indices.graphicsFamily.value(),
        // };

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++)
        {
            VK_VALIDATE(vkCreateCommandPool(device->GetVkDevice(), &commandPoolInfo, nullptr, &m_frameData[i].CommandPool));

            VkCommandBuffer commandBuffers[2];
            VkCommandBufferAllocateInfo commandBuffersAllocInfo =
            {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = m_frameData[i].CommandPool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 2
            };

            VK_VALIDATE(vkAllocateCommandBuffers(device->GetVkDevice(), &commandBuffersAllocInfo, commandBuffers));

            m_frameData[i].MainCommandBuffer = commandBuffers[0];
            m_frameData[i].GuiCommandBuffer = commandBuffers[1];

            m_cleanup.appendFunction([=]()
            {
                vkDestroyCommandPool(device->GetVkDevice(), m_frameData[i].CommandPool, nullptr);
            });

            m_mainCommandBuffer[i] = GfxVkCommandBuffer({
                .type = CommandBufferType::MAIN,
                .commandBuffer = m_frameData[i].MainCommandBuffer,
                .fence = VK_NULL_HANDLE,
                .waitSemaphore = m_frameData[i].ImageAvailableSemaphore,
                .signalSemaphore = m_frameData[i].PresentRenderFinishedSemaphore,
            });
            m_uiCommandBuffer[i] = GfxVkCommandBuffer({
                .type = CommandBufferType::UI,
                .commandBuffer = m_frameData[i].GuiCommandBuffer,
                .fence = VK_NULL_HANDLE,
                .waitSemaphore = m_frameData[i].PresentRenderFinishedSemaphore,
                .signalSemaphore = m_frameData[i].GuiRenderFinishedSemaphore,
            });
        }

        //TODO: change to streaming like above
        VkCommandPoolCreateInfo uploadCommandPoolInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = indices.graphicsFamily.value(),
        };

        VK_VALIDATE(vkCreateCommandPool(device->GetVkDevice(), &uploadCommandPoolInfo, nullptr, &m_upload.CommandPool));

        m_cleanup.appendFunction([=]()
        {
            vkDestroyCommandPool(device->GetVkDevice(), m_upload.CommandPool, nullptr);
        });

        VkCommandBufferAllocateInfo uploadCommandBufferAllocInfo =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_upload.CommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VK_VALIDATE(vkAllocateCommandBuffers(device->GetVkDevice(), &uploadCommandBufferAllocInfo, &m_upload.CommandBuffer));
    }

    void VulkanRenderer::createDescriptorPool()
    {  
        VulkanDevice* device = (VulkanDevice*)Device::instance;

        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
        };

        VkDescriptorPoolCreateInfo tDescriptorPoolInfo =
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 100,
            .poolSizeCount = std::size(poolSizes),
            .pPoolSizes = poolSizes,
        };

        VK_VALIDATE(vkCreateDescriptorPool
        (
            device->GetVkDevice(), 
            &tDescriptorPoolInfo, 
            nullptr,
            &m_descriptorPool
        )); 

        m_cleanup.appendFunction([=]()
            {
                vkDestroyDescriptorPool(device->GetVkDevice(), m_descriptorPool, nullptr);
            }
        );
    }

    VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
    {
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM 
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

