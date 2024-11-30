#include "platform/Vulkan/GfxVkRenderPassRenderer.h"
#include "platform/Vulkan/GfxVulkanDevice.h"
#include "platform/Vulkan/GfxVulkanRenderer.h"
#include "platform/Vulkan/resources/GfxVkResourceManager.h"

namespace Engine
{
    void GfxVkRenderPassRenderer::BeginRenderPass(Handle<RenderPass> renderPass, Handle<FrameBuffer> frameBuffer)
    {
        VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;
        VkResourceManager* rm = (VkResourceManager*)ResourceManager::instance;

        m_commandBuffer = renderer->BeginCommandRecording(m_stage, m_type);

        GfxVkRenderPass* vkRenderPass = rm->getRenderPass(renderPass);
        GfxVkFrameBuffer* vkFramebuffer = rm->getFrameBuffer(frameBuffer);

        std::vector<VkClearValue> clearValues;

    }

    void GfxVkRenderPassRenderer::EndRenderPass()
    {
        vkCmdEndRenderPass(m_commandBuffer->GetCommandBuffer());
        VK_VALIDATE(vkEndCommandBuffer(m_commandBuffer->GetCommandBuffer()));
    }

    void GfxVkRenderPassRenderer::Submit()
    {
        VulkanRenderer* renderer = (VulkanRenderer*)Renderer::instance;

        VkSubmitInfo submitInfo =
        {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pCommandBuffers = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_commandBuffer->GetSignalSemaphore(),
            .commandBufferCount = 1,
            .pCommandBuffers = &m_commandBuffer->GetCommandBuffer(),
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_commandBuffer->GetSignalSemaphore(),
        };

        VK_VALIDATE(vkQueueSubmit(renderer->GetGraphicsQueue(), 1, &submitInfo, m_commandBuffer->GetFence()));
    }

    void GfxVkRenderPassRenderer::Draw()
    {
    }
}