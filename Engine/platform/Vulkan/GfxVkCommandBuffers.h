#pragma once

#include "src/renderer/resources/Enums.h"
#include "platform/Vulkan/GfxVulkanCore.h"

namespace Engine
{
    struct GfxVkCommandBufferDesc
    {
        CommandBufferType type;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkFence fence = VK_NULL_HANDLE;
        VkSemaphore waitSemaphore = VK_NULL_HANDLE;
        VkSemaphore signalSemaphore = VK_NULL_HANDLE;
    };

    class GfxVkCommandBuffer
    {
    public:
        GfxVkCommandBuffer() = default;
        GfxVkCommandBuffer(const GfxVkCommandBufferDesc&& desc);

        CommandBufferType& GetType();
        VkCommandBuffer& GetCommandBuffer();
        VkFence& GetFence();
        VkSemaphore& GetWaitSemaphore();
        VkSemaphore& GetSignalSemaphore();

    private:
        CommandBufferType m_type = CommandBufferType::MAIN;
        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
        VkFence m_fence = VK_NULL_HANDLE;
        VkSemaphore m_waitSemaphore = VK_NULL_HANDLE;
        VkSemaphore m_signalSemaphore = VK_NULL_HANDLE;
    };
}