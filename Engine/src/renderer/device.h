#pragma once 

#include "core/core.h"
#include "core/assert.h"

namespace Engine
{
	struct DeviceProperties
	{
		std::string vendorId;
		std::string deviceName; 
		std::string driverVersion;
	}; 

	class Device
	{
	public: 
		static inline Device* instance = nullptr;

		virtual ~Device() = default;
	
		virtual void Init() = 0;
		virtual void ShutDown() = 0;   

	protected: 
		DeviceProperties m_properties; 
	};
}
