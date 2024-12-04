#pragma once

#include "src/renderer/resources/bindGroups.h" 

namespace Engine
{
	class GfxVkBindGroup
	{  
	public:  
		GfxVkBindGroup() = default; 
		GfxVkBindGroup(const BindGroupDesc&& desc) {} 
		void destroy() {}
	private: 

	};
}