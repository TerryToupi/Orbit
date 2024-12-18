#include "platform/Vulkan/GfxVkCommandBuffers.h" 
#include "platform/Vulkan/GfxVkRenderPassRenderer.h"
#include "platform/Vulkan/GfxVulkanDevice.h"
#include "platform/Vulkan/GfxVulkanRenderer.h"
#include "platform/Vulkan/resources/GfxVkResourceManager.h"

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

    RenderPassRenderer* GfxVkCommandBuffer::BeginRenderPass(RENDERPASS renderPass, FRAMEBUFFER frameBuffer)
    {
        VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;
        VkResourceManager* rm = (VkResourceManager*)ResourceManager::instance;

        ENGINE_ASSERT(m_state == CommandBufferState::COMMAND_BUFFER_STATE_RECORDING); 

        m_renderPassRenderer.SetActiveBuffer(m_commandBuffer);

        GfxVkRenderPass* vkRenderPass = rm->getRenderPass(renderPass);
        GfxVkFrameBuffer* vkFramebuffer = rm->getFrameBuffer(frameBuffer);

        std::vector<VkClearValue> clearValues;

        std::vector<bool> clValues = vkRenderPass->GetColorClearValues();
        for (bool clVlaue : clValues)
        {
            if (clVlaue)
            {
                VkClearValue vkCl = {
                    .color = { { 0.1f, 0.1f, 0.1f, 1.0f } }
                };

                clearValues.push_back(vkCl);
            }
        }

        if (vkRenderPass->GetDepthClearValue())
        {
            VkClearValue vkCl;
            vkCl.depthStencil.depth = 1.0f;
            clearValues.push_back(vkCl);
        }

        VkRenderPassBeginInfo rpInfo =
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = vkRenderPass->GetRenderPass(),
            .framebuffer = vkFramebuffer->GetFrameBuffer(),
            .renderArea =
            {
                .offset = {0, 0},
                .extent = { vkFramebuffer->GetWidth(), vkFramebuffer->GetHeight() },
            },
            .clearValueCount = (uint32_t)clearValues.size(),
            .pClearValues = clearValues.data(),
        };

        vkCmdBeginRenderPass(m_commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_state = CommandBufferState::COMMAND_BUFFER_STATE_IN_RENDER_PASS; 

        return &m_renderPassRenderer;
    }

    void GfxVkCommandBuffer::EndRenderPass(const RenderPassRenderer* renderPassRenderer)
    {  
        ENGINE_ASSERT(renderPassRenderer == &m_renderPassRenderer);

        ENGINE_ASSERT(m_state == CommandBufferState::COMMAND_BUFFER_STATE_IN_RENDER_PASS);
        vkCmdEndRenderPass(m_commandBuffer);
        VK_VALIDATE(vkEndCommandBuffer(m_commandBuffer));
        m_state = CommandBufferState::COMMAND_BUFFER_STATE_RECORDING_ENDED;
    }

    void GfxVkCommandBuffer::Submit()
    { 
        VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;

        ENGINE_ASSERT(m_state == CommandBufferState::COMMAND_BUFFER_STATE_RECORDING_ENDED);

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_waitSemaphore,
            .pWaitDstStageMask = &waitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = &m_commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_signalSemaphore,
        };

        VK_VALIDATE(vkQueueSubmit(renderer->GetGraphicsQueue(), 1, &submitInfo, m_fence));
        m_state = CommandBufferState::COMMAND_BUFFER_STATE_READY;
    }

    const CommandBufferState GfxVkCommandBuffer::GetState() const
    {
        return m_state;
    }

    void GfxVkCommandBuffer::SetState(const CommandBufferState& state)
    {
        m_state = state;
    }

    const CommandBufferType& GfxVkCommandBuffer::GetType() 
    {
        return m_type;
    }

    const VkCommandBuffer& GfxVkCommandBuffer::GetCommandBuffer()
    {
        return m_commandBuffer;
    }

    const VkFence& GfxVkCommandBuffer::GetFence()
    {
       return m_fence;
    }

    const VkSemaphore& GfxVkCommandBuffer::GetWaitSemaphore()
    {
        return m_waitSemaphore;
    }

   const VkSemaphore& GfxVkCommandBuffer::GetSignalSemaphore()
    {
        return m_signalSemaphore;
    }
}
