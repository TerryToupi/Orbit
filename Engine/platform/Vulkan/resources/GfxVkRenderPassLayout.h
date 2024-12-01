#pragma once 

#include "src/renderer/resources/renderPassLayouts.h"
#include <vector>

namespace Engine
{
	class GfxVkRenderPassLayout
	{
	public:
		GfxVkRenderPassLayout(const RenderPassLayoutDesc&& desc);

		void destroy();

		const char* getDebugName() const;
		TextureFormat& getDepthStencilFormat();
		std::vector<SubPass>& getSubPasses();

	private:
		const char* m_debugName;
		TextureFormat m_depthFormat;
		std::vector<SubPass> m_subPasses;
	};
}