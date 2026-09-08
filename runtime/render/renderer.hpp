#ifndef __ORBIT_RENDERER__
#define __ORBIT_RENDERER__

#include <utils/types.hpp>
#include <utils/containers.hpp>

namespace Orbit::GFX 
{
	static constexpr u64 kMaxDeviceCount = 32;

	void init();
	void shutdown();

	class Device;
	Handle<Device>  create_device();
	void 		    destroy_device(Handle<Device> h);
};


#endif