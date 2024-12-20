#pragma once

#include "src/renderer/renderPassRenderer.h" 
#include "platform/Vulkan/GfxVulkanCore.h"

namespace Engine
{
    class GfxVkRenderPassRenderer final : public RenderPassRenderer
    {
    public: 
        GfxVkRenderPassRenderer() = default;

        virtual void Draw() override {};

        void SetActiveBuffer(const VkCommandBuffer& _activeBuffer);

    private:
        VkCommandBuffer m_activeBuffer = VK_NULL_HANDLE;
    };
}