#pragma once 

#include "src/renderer/resources/Enums.h"
#include <span>

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

		std::span<const SubPass> subPasses;
	};
}
