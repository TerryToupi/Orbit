#ifndef __ORBIT_WINDOW__
#define __ORBIT_WINDOW__

#include <utils/types.hpp>
#include <utils/containers.hpp>

namespace Orbit::Window 
{
	static constexpr u64 kMaxWindowCount = 64;

	void init();
	void shutdown();

	class Window;
	Handle<Window>  create(const char *name, u64 width, u64 height);
	void 		    destroy(Handle<Window> h);
	void  		   *native(Handle<Window> h);
}

#endif