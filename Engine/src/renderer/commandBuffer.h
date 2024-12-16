#pragma once 

#include "src/renderer/resources/Enums.h"
#include "src/utilities/handles.h"
#include "src/renderer/resources/RenderPasses.h"
#include "src/renderer/resources/FrameBuffers.h" 
#include "src/renderer/renderPassRenderer.h"

namespace Engine
{
	class CommandBuffer
	{
	public: 
		virtual RenderPassRenderer* BeginRenderPass(RENDERPASS renderPass, FRAMEBUFFER frameBuffer) = 0;
		virtual void EndRenderPass(const RenderPassRenderer* renderPassRenderer) = 0;
		virtual void Submit() = 0;
	};
}