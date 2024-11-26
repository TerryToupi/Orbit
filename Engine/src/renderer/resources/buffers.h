#pragma once 

#include "src/renderer/resources/enums.h"

namespace Engine
{
	class Buffer;

	struct BufferDesc
	{
		const char* debugName;
		BufferUsage usage = BufferUsage::UNIFORM; 
		BufferUsageHint usageHint = BufferUsageHint::STATIC; 
		MemoryUsage memoryUsage = MemoryUsage::CPU_GPU;
		int size = 0;
		void* data = nullptr;
	}; 
}
