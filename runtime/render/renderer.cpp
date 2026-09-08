#include "SDL3/SDL_gpu.h"
#include <render/renderer.hpp>

#include <SDL3/SDL.h>

namespace Orbit::GFX 
{
	struct GPUResource
	{
		u64 kind;
		union	
		{
			SDL_GPUBuffer  *buffer;
			SDL_GPUTexture *texture;
			SDL_GPUSampler *sampler;
		};
	};

	struct DeviceContext
	{
		DeviceContext() = default;
		~DeviceContext() = default;
			
		SDL_GPUDevice     *device;
		Pool<GPUResource>  resources;
	};

	static StaticPool<DeviceContext, kMaxDeviceCount> pDeviceManager;

	void 
	init()
	{
		SDL_Log("[Orbit] GPU device manager initialized");
	}

	void
	shutdown()
	{
		// TODO(TPS): shutdown all of the active devices
	}

	Handle<Device>  
	create_device()
	{
		return Handle<Device>(0, 0);
	}

	void destroy_device(Handle<Device> h)
	{

	}
};