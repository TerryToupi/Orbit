#include "SDL3/SDL_stdinc.h"
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <rpmalloc.h>

#include <scene.h>

struct component
{
    std::byte data[1024];
};

struct AppState
{
    SDL_Window      *window = nullptr;
    SDL_Renderer    *renderer = nullptr;
};

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

void dump_memory()
{
    rpmalloc_global_statistics_t stats{};
    rpmalloc_global_statistics(&stats);

    SDL_Log("rpmalloc memory:");
    SDL_Log("  mapped:          %zu MB",
        stats.mapped / (1024 * 1024));

    SDL_Log("  mapped peak:     %zu MB",
        stats.mapped_peak / (1024 * 1024));
    
    SDL_Log("  commited peak:     %zu MB",
        stats.committed / (1024 * 1024));

    SDL_Log("  huge allocated:  %zu MB",
        stats.huge_alloc / (1024 * 1024));

    SDL_Log("  huge peak:       %zu MB",
        stats.huge_alloc_peak / (1024 * 1024));
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    rpmalloc_config_t rp_config = {
        .enable_huge_pages = 1
    };

    if (rpmalloc_initialize_config(nullptr, &rp_config) != 0)
        return SDL_APP_FAILURE;

    /*
     * Tell SDL to use rpmalloc.
     *
     * Do this BEFORE SDL_Init().
     */

    if (!SDL_SetMemoryFunctions(
        sdl_malloc,
        sdl_calloc,
        sdl_realloc,
        sdl_free))
    {
        rpmalloc_finalize();
        return SDL_APP_FAILURE;
    }


    // -----------------------------------------------------
    // SDL
    // -----------------------------------------------------

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        rpmalloc_finalize();
        return SDL_APP_FAILURE;
    }

    auto* state = (AppState*)SDL_malloc(sizeof(AppState));
    
    Registry world;
    
    state->window = SDL_CreateWindow(
        "SDL3 Callbacks",
        1280,
        720,
        SDL_WINDOW_RESIZABLE
    );

    if (!state->window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_free(appstate);
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    state->renderer = SDL_CreateRenderer(
        state->window,
        nullptr
    );

    if (!state->renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(state->window);
        SDL_free(appstate);
        SDL_Quit();
        return SDL_APP_FAILURE;
    }

    *appstate = state;
    
    auto player = world.create();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    dump_memory();

    auto* state = static_cast<AppState*>(appstate);

    SDL_SetRenderDrawColor(
        state->renderer,
        20,
        20,
        20,
        255
    );

    SDL_RenderClear(state->renderer);

    SDL_FRect rect{
        .x = 100.0f,
        .y = 100.0f,
        .w = 200.0f,
        .h = 200.0f
    };

    SDL_SetRenderDrawColor(
        state->renderer,
        255,
        120,
        40,
        255
    );

    SDL_RenderFillRect(
        state->renderer,
        &rect
    );

    SDL_RenderPresent(state->renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    auto* state = static_cast<AppState*>(appstate);

    if (state) {
        SDL_DestroyRenderer(state->renderer);
        SDL_DestroyWindow(state->window);
        SDL_free(state);
    }

    SDL_Quit();
}
