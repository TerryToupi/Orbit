#include "platform/Vulkan/GfxVkRenderPassRenderer.h"
#include "platform/Vulkan/GfxVulkanDevice.h"
#include "platform/Vulkan/GfxVulkanRenderer.h"
#include "platform/Vulkan/resources/GfxVkResourceManager.h"

namespace Engine
{
    void GfxVkRenderPassRenderer::SetActiveBuffer(const VkCommandBuffer& _activeBuffer)
    { 
        ENGINE_ASSERT(_activeBuffer != VK_NULL_HANDLE);  
        
        m_activeBuffer = _activeBuffer;
    }
}