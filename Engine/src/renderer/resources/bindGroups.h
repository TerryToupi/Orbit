#pragma once 

#include "src/utilities/handles.h"

#define BINDGROUP Handle<BindGroup>

namespace Engine
{
	class BindGroup;

	struct BindGroupDesc
	{
		int data;
	};
}
