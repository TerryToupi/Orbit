#include <window/window.hpp>

#include <SDL3/SDL.h>

namespace Orbit::Window
{
	static StaticPool<SDL_Window*, kMaxWindowCount> pWindowManager;

	void 
	init()
	{
		SDL_Log("[Orbit] Window Manager initialized");
	}

	void 
	shutdown()
	{
		// TODO(TPS): destroy all windows;
	}

	Handle<Window> 
	create(const char *name, u64 width, u64 height)
	{
		auto h = pWindowManager.emplace(SDL_CreateWindow(name, width, height, SDL_WINDOW_RESIZABLE));
		return Handle<Window>(h.idx, h.gen);
	}

	void *
	native(Handle<Window> h)
	{
		SDL_Window *w = *pWindowManager.at({h.idx(), h.gen()});
		return reinterpret_cast<void*>(w); 
	}

	void 
	destroy(Handle<Window> h)
	{
		SDL_Window *w = *pWindowManager.at({h.idx(), h.gen()});
		assert(w);
		SDL_DestroyWindow(w);
		pWindowManager.erase({h.idx(), h.gen()});
	}
} 