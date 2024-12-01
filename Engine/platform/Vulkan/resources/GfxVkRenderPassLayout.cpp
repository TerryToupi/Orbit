#include "platform/Vulkan/resources/GfxVkRenderPassLayout.h"

namespace Engine
{
    GfxVkRenderPassLayout::GfxVkRenderPassLayout(const RenderPassLayoutDesc&& desc)
    {
        m_debugName = desc.debugName;
        m_depthFormat = desc.depthTargetFormat;

        for (const auto& subpass : desc.subPasses)
        {
            m_subPasses.push_back(subpass);
        }
    }

    const char* GfxVkRenderPassLayout::getDebugName() const
    {
        return m_debugName;
    }

    TextureFormat& GfxVkRenderPassLayout::getDepthStencilFormat()
    {
        return m_depthFormat;
    }

    std::vector<SubPass>& GfxVkRenderPassLayout::getSubPasses()
    {
       return m_subPasses;
    }
}