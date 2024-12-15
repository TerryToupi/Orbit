#pragma once 

#include "src/utilities/handles.h"
#include "src/renderer/resources/Enums.h"
#include "src/renderer/resources/Textures.h"
#include "src/renderer/resources/RenderPasses.h" 

#include <initializer_list>

#define FRAMEBUFFER Handle<FrameBuffer>

namespace Engine
{
	class FrameBuffer;

	struct FrameBufferDesc
	{
		const char* debugName; 
		bool resizable = true;
		uint32_t width = 0;
		uint32_t height = 0;
		RENDERPASS renderPass;
		TEXTURE depthTarget;
		std::initializer_list<TEXTURE> colorTargets;
	};
}
