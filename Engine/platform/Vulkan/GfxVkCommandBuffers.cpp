#include "platform/Vulkan/GfxVkCommandBuffers.h"

namespace Engine
{
    GfxVkCommandBuffer::GfxVkCommandBuffer(const GfxVkCommandBufferDesc&& desc)
        :   m_type(desc.type),
            m_state(desc.state),
            m_commandBuffer(desc.commandBuffer),
            m_fence(desc.fence),
            m_waitSemaphore(desc.waitSemaphore),
            m_signalSemaphore(desc.signalSemaphore)
    {
    }

    CommandBufferState GfxVkCommandBuffer::GetState() const
    {
        return m_state;
    }

    void GfxVkCommandBuffer::SetState(const CommandBufferState& state)
    {
        m_state = state;
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
