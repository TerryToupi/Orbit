#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory_setup.h>

#include <thread_context.h>
#include <servers.h>
#include <asset_factory.h>
#include <render/rendering_compositor.h>
#include <memory>

// Single borrowed preview mesh until the actual Scene implementation exists.
struct PreviewScene final : Scene
{
    SceneMesh instance = {};
    SceneMatrix camera = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    Span<SceneMesh> meshes() const override { return {&instance, 1}; }
    const SceneMatrix& view_projection() const override { return camera; }
};

struct AppState
{
    ThreadContext   thread_context = {};
    Servers         servers = {};
    AssetFactory    assets = {};
    SDL_Window      *window = nullptr;
    RenderingCompositor compositor = {};
    PreviewScene    scene = {};
    AssetHandle<MeshAsset> preview = {};
};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!initialize_memory())
        return SDL_APP_FAILURE;
    if (!SDL_Init(SDL_INIT_VIDEO))
        return SDL_APP_FAILURE;

    AppState* state = static_cast<AppState*>(SDL_malloc(sizeof(AppState)));
    std::construct_at(state);
    *appstate = state;
    if (!set_thread_context(&state->thread_context)) {
        SDL_Log("Thread context initialization failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!create_servers(state->servers)) {
        SDL_Log("Server initialization failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!create_asset_factory(state->assets, state->servers.artifacts, ".")) {
        SDL_Log("Asset factory initialization failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    state->window = SDL_CreateWindow(
        "ORbit",
        1280,
        720,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if (!state->window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!create_rendering_compositor(state->compositor, state->window)) {
        SDL_Log("Rendering initialization failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (argc > 1)
        state->preview = asset_load(state->assets, argv[1]);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void*, SDL_Event* event)
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
    AppState* state = static_cast<AppState*>(appstate);

    ArtifactEvent event;
    while (command_next_event(state->servers.artifacts.commands, event))
        assets_process_event(state->assets, event);
    assets_tick(state->assets);

    if (asset_valid(state->assets, state->preview))
        state->scene.instance.mesh = asset_get(state->assets, state->preview);
    if (!state->compositor.render(state->scene)) {
        SDL_Log("Rendering failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult)
{
    AppState* state = static_cast<AppState*>(appstate);

    if (state) {
        destroy_rendering_compositor(state->compositor);
        destroy_asset_factory(state->assets);
        destroy_servers(state->servers);
        SDL_DestroyWindow(state->window);
        destroy_thread_context(state->thread_context);
        std::destroy_at(state);
        SDL_free(state);
    }

    SDL_Quit();
}
