#pragma once

#include "src/renderer/renderPassRenderer.h"
#include "platform/Vulkan/GfxVkCommandBuffers.h"

namespace Engine
{
    class GfxVkRenderPassRenderer final : public RenderPassRenderer
    {
    public:
        GfxVkRenderPassRenderer(const RenderPassStage stage, const CommandBufferType type)
            : RenderPassRenderer(stage, type), m_commandBuffer(VK_NULL_HANDLE)
        {
        }

        virtual void BeginRenderPass(RENDERPASS renderPass, FRAMEBUFFER frameBuffer) override;
        virtual void EndRenderPass() override;
        virtual void Submit() override;
        virtual void Draw() override;

    private:
        GfxVkCommandBuffer* m_commandBuffer;
    };
}