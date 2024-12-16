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
        virtual void Draw() = 0;
    };
}