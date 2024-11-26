#pragma once

#include "src/renderer/resources/enums.h" 
#include "float.h"

namespace Engine
{
	class Sampler;

	struct SamplerDesc
	{  
		struct Filters 
		{
			Filter min = Filter::NEAREST; 
			Filter mag = Filter::NEAREST;
			Filter mip = Filter::NEAREST;
		};  

		Filters filters;
		Compare compareType = Compare::LESS_OR_EQUAL; 
		Wrap wrap = Wrap::REPEAT;
		bool compare = false; 
		float minMip = 0.0f;
		float maxMip = FLT_MAX; 
		float lodBias = 0.0f;
	};
}