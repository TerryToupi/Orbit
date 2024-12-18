#pragma once 

#include<glm/glm.hpp>

#include "src/utilities/handles.h"
#include "src/renderer/resources/Enums.h"
#include "src/renderer/resources/RenderPassLayouts.h" 
#include "src/utilities/handles.h"
#include "src/utilities/span.h" 

#define RENDERPASS Handle<RenderPass>

namespace Engine
{
	class RenderPass;

	struct RenderPassDesc
	{ 
		struct ColorTarget
		{ 
			TextureFormat format = TextureFormat::BACKBUFFER;
			LoadOperation loadOp = LoadOperation::CLEAR;
			StoreOperation storeOp = StoreOperation::STORE;
			TextureLayout prevUsage = TextureLayout::UNDEFINED;
			TextureLayout nextUsage = TextureLayout::UNDEFINED;
			glm::vec4 clearColor = glm::vec4(0.0f);
		};

		struct DepthTarget
		{
			LoadOperation loadOp = LoadOperation::CLEAR;
			StoreOperation storeOp = StoreOperation::STORE;
			LoadOperation stencilLoadOp = LoadOperation::CLEAR;
			StoreOperation stencilStoreOp = StoreOperation::STORE;
			TextureLayout prevUsage = TextureLayout::UNDEFINED;
			TextureLayout nextUsage = TextureLayout::UNDEFINED;
			float clearZbuffer = 0.0f;
			uint32_t clearStencil = 0;
		};

		const char* debugName;
		Handle<RenderPassLayout> layout;
		DepthTarget depthTarget;
		Span<const ColorTarget> colorTargets;
	};
}
