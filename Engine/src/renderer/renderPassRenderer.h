#pragma once

#include "src/renderer/resources/Enums.h"
#include "src/utilities/handles.h"
#include "src/renderer/resources/RenderPasses.h"
#include "src/renderer/resources/FrameBuffers.h"

namespace Engine
{
    struct RenderPassDesc
    {
        RenderPassStage stage;
        CommandBufferType type;
    };

    class RenderPassRenderer
    {
    public:
        virtual ~RenderPassRenderer() = default;

        virtual void BeginRenderPass(Handle<RenderPass> renderPass, Handle<FrameBuffer> frameBuffer) = 0;
        virtual void EndRenderPass() = 0;
        virtual void Submit() = 0;
        virtual void Draw() = 0;
        virtual void DrawIndexed() = 0;

    private:
        RenderPassStage m_stage;
        CommandBufferType m_type;
    };
}