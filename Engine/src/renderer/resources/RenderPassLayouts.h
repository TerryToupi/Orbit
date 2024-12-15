#pragma once 

#include "src/utilities/handles.h"
#include "src/renderer/resources/Enums.h"
#include "src/utilities/span.h"

#define RENDERPASSLAYOUT Handle<RenderPassLayout>

namespace Engine
{
	class RenderPassLayout;

	struct SubPass
	{
		bool depthTarget = false;
		uint32_t colorTargets = 0;
	};

	struct RenderPassLayoutDesc
	{
		const char* debugName;
		TextureFormat depthTargetFormat = TextureFormat::D32_FLOAT;

		Span<const SubPass> subPasses;
	};
}
