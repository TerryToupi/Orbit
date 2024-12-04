#pragma once 

#include "src/renderer/resources/Enums.h"
#include "src/renderer/resources/Textures.h"
#include "src/renderer/resources/RenderPasses.h"
#include <initializer_list>

namespace Engine
{
	class FrameBuffer;

	struct FrameBufferDesc
	{
		const char* debugName; 
		bool resizable = true;
		uint32_t width = 0;
		uint32_t height = 0;
		Handle<RenderPass> renderPass;
		Handle<Texture> depthTarget;
		std::initializer_list<Handle<Texture>> colorTargets;
	};
}
