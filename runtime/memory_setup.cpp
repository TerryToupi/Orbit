#include <memory_setup.h>
#include <rpmalloc.h>
#include <SDL3/SDL.h>

static void* sdl_malloc(size_t size)
{
    return rpmalloc(size);
}

static void* sdl_calloc(size_t count, size_t size)
{
    return rpcalloc(count, size);
}

static void* sdl_realloc(void* ptr, size_t size)
{
    return rprealloc(ptr, size);
}

static void sdl_free(void* ptr)
{
    rpfree(ptr);
}

bool initialize_memory()
{
    rpmalloc_config_t config = {.enable_huge_pages = 1};
    if (rpmalloc_initialize_config(nullptr, &config) != 0) {
        config.enable_huge_pages = 0;
        if (rpmalloc_initialize_config(nullptr, &config) != 0)
            return SDL_SetError("Memory allocator initialization failed");
    }
    if (!SDL_SetMemoryFunctions(sdl_malloc, sdl_calloc, sdl_realloc, sdl_free)) {
        rpmalloc_finalize();
        return false;
    }
    return true;
}

void dump_memory()
{
    rpmalloc_global_statistics_t stats{};
    rpmalloc_global_statistics(&stats);
    SDL_Log("rpmalloc memory:");
    SDL_Log("  mapped:          %zu MB", stats.mapped / (1024 * 1024));
    SDL_Log("  mapped peak:     %zu MB", stats.mapped_peak / (1024 * 1024));
    SDL_Log("  commited peak:     %zu MB", stats.committed / (1024 * 1024));
    SDL_Log("  huge allocated:  %zu MB", stats.huge_alloc / (1024 * 1024));
    SDL_Log("  huge peak:       %zu MB", stats.huge_alloc_peak / (1024 * 1024));
}
