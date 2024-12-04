#pragma once 

#include "src/renderer/resources/shaders.h"

namespace Engine
{
	class GfxVkShader
	{ 
	public: 
		GfxVkShader() = default;
		GfxVkShader(const ShaderDesc&& desc) {} 
		void destroy() {}
	private:
	};
}