#include "platform/Vulkan/GfxVkCommandBuffers.h"

namespace Engine
{
    GfxVkCommandBuffer::GfxVkCommandBuffer(const GfxVkCommandBufferDesc&& desc)
    {
        m_type = desc.type;
        m_commandBuffer = desc.commandBuffer;
        m_fence = desc.fence;
        m_waitSemaphore = desc.waitSemaphore;
        m_signalSemaphore = desc.signalSemaphore;
    }

    CommandBufferType& GfxVkCommandBuffer::GetType()
    {
        return m_type;
    }

    VkCommandBuffer& GfxVkCommandBuffer::GetCommandBuffer()
    {
        return m_commandBuffer;
    }

    VkFence& GfxVkCommandBuffer::GetFence()
    {
       return m_fence;
    }

    VkSemaphore& GfxVkCommandBuffer::GetWaitSemaphore()
    {
        return m_waitSemaphore;
    }

    VkSemaphore& GfxVkCommandBuffer::GetSignalSemaphore()
    {
        return m_signalSemaphore;
    }
}
