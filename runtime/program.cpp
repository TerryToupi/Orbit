#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory_setup.h>

#include <thread_context.h>
#include <servers.h>
#include <asset_factory.h>
#include <render/rendering_compositor.h>
#include <render/render_asset_cache.h>
#include <memory>

struct AppState
{
    ThreadContext   thread_context = {};
    Servers         servers = {};
    AssetFactory    assets = {};
    SDL_Window      *window = nullptr;
    RenderingCompositor compositor = {};
    RenderAssetCache render_assets = {};
    GPUTransferQueue transfers = {};
    GeometryDraw    preview_draw = {};
    Mat4            view = {};
    Mat4            camera = {};
    uint64_t        asset_refresh = 0;
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

    if (!create_rendering_compositor(state->compositor, state->window, state->servers.artifacts)) {
        SDL_Log("Rendering initialization failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    state->render_assets.assets = &state->assets;
    state->render_assets.device = state->compositor.device;
    state->preview_draw.world = make_transform_matrix(b3Transform_identity);
    state->view = make_view_matrix({.p = {.z = 3}, .q = b3Quat_identity});
    int width, height;
    if (!SDL_GetWindowSizeInPixels(state->window, &width, &height))
        return SDL_APP_FAILURE;
    state->camera = mul(make_perspective(B3_PI / 3, float(width) / float(height), .1f, 1000), state->view);
    if (argc > 1) {
        state->preview = asset_load(state->assets, argv[1]);
        if (asset_valid(state->assets, state->preview))
            request_render_mesh(state->render_assets, asset_status(state->assets, state->preview).id);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    AppState* state = static_cast<AppState*>(appstate);
    if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED && event->window.data1 > 0 && event->window.data2 > 0) {
        state->camera = mul(make_perspective(B3_PI / 3, float(event->window.data1) / float(event->window.data2), .1f, 1000), state->view);
    }
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
    while (command_next_event(state->servers.artifacts.commands, event)) {
        if (rendering_process_event(state->compositor, event)) {
            if (state->compositor.shader_failed && !state->compositor.depth_pass.pipeline)
                return SDL_APP_FAILURE;
        } else {
            assets_process_event(state->assets, event);
        }
    }
    render_assets_process_changes(state->render_assets);
    assets_tick(state->assets);
    uint64_t now = SDL_GetTicks();
    if (now >= state->asset_refresh) {
        reload_rendering_shaders(state->compositor);
        if (asset_valid(state->assets, state->preview) && !asset_status(state->assets, state->preview).loading)
            asset_reload(state->assets, state->preview);
        state->asset_refresh = now + 500;
    }
    rendering_shaders_tick(state->compositor);

    prepare_render_assets(state->render_assets, state->transfers);
    state->preview_draw.mesh = render_mesh(state->render_assets, state->preview);
    bool rendered = state->compositor.render(state->transfers, {&state->preview_draw, 1}, state->camera);
    finish_render_assets(state->render_assets, state->transfers);
    if (!rendered) {
        SDL_Log("Rendering failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult)
{
    AppState* state = static_cast<AppState*>(appstate);

    if (state) {
        if (state->compositor.device) {
            SDL_WaitForGPUIdle(state->compositor.device);
            destroy_render_asset_cache(state->render_assets);
            destroy_gpu_transfer_queue(state->transfers, state->compositor.device);
        }
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
