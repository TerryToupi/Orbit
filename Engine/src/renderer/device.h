#pragma once 

#include <core/core.h>   
#include <core/assert.h> 

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
		virtual ~Device() = default;
	
		template <typename T>
		static T& Get()
		{ 
			return static_cast<T&>(*s_instance);
		}

		static void Create(); 
		static void Destroy();

	protected:
		virtual void Init() = 0;
		virtual void ShutDown() = 0;   

	protected: 
		DeviceProperties m_properties; 
	
	private:
		static Device* s_instance;
	};
}
