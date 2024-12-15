#pragma once

#include "src/renderer/resources/Enums.h"
#include "src/utilities/handles.h"
#include "src/renderer/resources/RenderPasses.h"
#include "src/renderer/resources/FrameBuffers.h"

namespace Engine
{
    class RenderPassRenderer
    {
    public:
        RenderPassRenderer(const RenderPassStage stage, const CommandBufferType type)
            : m_stage(stage), m_type(type)
        {
        }

        virtual ~RenderPassRenderer() = default;

        virtual void BeginRenderPass(RENDERPASS renderPass, FRAMEBUFFER frameBuffer) = 0;
        virtual void EndRenderPass() = 0;
        virtual void Submit() = 0;
        virtual void Draw() = 0;

    protected:
        RenderPassStage m_stage;
        CommandBufferType m_type;
    };
}