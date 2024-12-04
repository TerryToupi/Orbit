#pragma once 

#include "src/renderer/resources/bindGroupLayout.h" 

namespace Engine
{
	class GfxVkBindGroupLayout
	{  
	public:  
		GfxVkBindGroupLayout() = default; 
		GfxVkBindGroupLayout(const BindGroupLayoutDesc&& desc) {} 

		void destroy() {}
	private:
	};
}