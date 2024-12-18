#pragma once

#include "src/renderer/resources/Enums.h"
#include "platform/Vulkan/GfxVulkanCore.h"  
#include "src/renderer/commandBuffer.h"
#include "platform/Vulkan/GfxVkRenderPassRenderer.h"

namespace Engine
{
    struct GfxVkCommandBufferDesc
    {
        CommandBufferType type;
        CommandBufferState state;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkFence fence = VK_NULL_HANDLE;
        VkSemaphore waitSemaphore = VK_NULL_HANDLE;
        VkSemaphore signalSemaphore = VK_NULL_HANDLE;
    };

    class GfxVkCommandBuffer final : public CommandBuffer
    {
    public:
        GfxVkCommandBuffer() = default;
        GfxVkCommandBuffer(const GfxVkCommandBufferDesc&& desc); 

        virtual RenderPassRenderer* BeginRenderPass(RENDERPASS renderPass, FRAMEBUFFER frameBuffer) override;
        virtual void EndRenderPass(const RenderPassRenderer* renderPassRenderer) override;
        virtual void Submit() override;

        const CommandBufferType& GetType();
        const VkCommandBuffer& GetCommandBuffer();
        const VkFence& GetFence();
        const VkSemaphore& GetWaitSemaphore();
        const VkSemaphore& GetSignalSemaphore(); 

        const CommandBufferState GetState() const;
        void SetState(const CommandBufferState& state); 

    private:
        //TODO: possibly we have to change renderpass renderer to be MT and request one from the renerer if state ready
        GfxVkRenderPassRenderer m_renderPassRenderer;
        CommandBufferType m_type = CommandBufferType::MAIN;
        CommandBufferState m_state = CommandBufferState::COMMAND_BUFFER_STATE_NOT_ALLOCATED;
        VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
        VkFence m_fence = VK_NULL_HANDLE;
        VkSemaphore m_waitSemaphore = VK_NULL_HANDLE;
        VkSemaphore m_signalSemaphore = VK_NULL_HANDLE; 
    };
}