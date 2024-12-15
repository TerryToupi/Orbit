#pragma once

#include "src/utilities/handles.h"

#define BINDGROUPLAYOUT Handle<BindGroupLayout>

namespace Engine
{
	class BindGroupLayout;

	struct BindGroupLayoutDesc
	{
		int data;
	};
}