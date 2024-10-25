#pragma once 

#include <core/core.h>   
#include <core/assert.h>

namespace Engine
{
	struct DeviceProperties
	{

	}; 

	class Device
	{
	public:
		virtual ~Device() = default;

		static Device& Get(); 
		static void Create(); 
		static void Destroy();

	protected:
		virtual void Init() = 0;
		virtual void ShutDown() = 0;   

	private: 
		DeviceProperties m_properties; 
	
	private:
		static Device* s_instance;
	};
}
