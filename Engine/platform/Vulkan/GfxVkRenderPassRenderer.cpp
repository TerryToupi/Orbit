#include "platform/Vulkan/GfxVkRenderPassRenderer.h"
#include "platform/Vulkan/GfxVulkanDevice.h"
#include "platform/Vulkan/GfxVulkanRenderer.h"
#include "platform/Vulkan/resources/GfxVkResourceManager.h"

namespace Engine
{
    void GfxVkRenderPassRenderer::BeginRenderPass(RENDERPASS renderPass, FRAMEBUFFER frameBuffer)
    {
        VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;
        VkResourceManager* rm = (VkResourceManager*)ResourceManager::instance;

        m_commandBuffer = renderer->BeginCommandRecording(m_stage, m_type);
        ENGINE_ASSERT(m_commandBuffer->GetState() == CommandBufferState::COMMAND_BUFFER_STATE_RECORDING);

        GfxVkRenderPass* vkRenderPass = rm->getRenderPass(renderPass);
        GfxVkFrameBuffer* vkFramebuffer = rm->getFrameBuffer(frameBuffer);

        std::vector<VkClearValue> clearValues; 

        for (auto clVlaue : vkRenderPass->GetColorClearValues())
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

        vkCmdBeginRenderPass(m_commandBuffer->GetCommandBuffer(), &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_commandBuffer->SetState(CommandBufferState::COMMAND_BUFFER_STATE_IN_RENDER_PASS);
    }

    void GfxVkRenderPassRenderer::EndRenderPass()
    {
        ENGINE_ASSERT(m_commandBuffer->GetState() == CommandBufferState::COMMAND_BUFFER_STATE_IN_RENDER_PASS);
        vkCmdEndRenderPass(m_commandBuffer->GetCommandBuffer());
        VK_VALIDATE(vkEndCommandBuffer(m_commandBuffer->GetCommandBuffer()));
        m_commandBuffer->SetState(CommandBufferState::COMMAND_BUFFER_STATE_RECORDING_ENDED);
    }

    void GfxVkRenderPassRenderer::Submit()
    {
        VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance; 

        ENGINE_ASSERT(m_commandBuffer->GetState() == CommandBufferState::COMMAND_BUFFER_STATE_RECORDING_ENDED);

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submitInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_commandBuffer->GetSignalSemaphore(),
            .pWaitDstStageMask = &waitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = &m_commandBuffer->GetCommandBuffer(),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_commandBuffer->GetSignalSemaphore(),
        };

        VK_VALIDATE(vkQueueSubmit(renderer->GetGraphicsQueue(), 1, &submitInfo, m_commandBuffer->GetFence()));
        m_commandBuffer->SetState(CommandBufferState::COMMAND_BUFFER_STATE_SUBMITTED);
    }

    void GfxVkRenderPassRenderer::Draw()
    {
    }
}