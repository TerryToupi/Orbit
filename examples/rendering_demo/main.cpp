#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory_setup.h>
#include <path.h>
#include <servers.h>
#include <thread_context.h>
#include <render/render_asset_cache.h>
#include <render/rendering_compositor.h>
#include <cfloat>
#include <memory>

static const char* const demo_mesh_names[] = {
    "Backplate Khronos", "Band Carbon Fiber", "Band Plastic", "Bezel Frame", "Button Metal", "Button Plastic", "Clasp DGG",
    "Glass Face", "Hand Hours", "Hand Minutes", "Hand Seconds", "Hand Setting", "Watch Face"
};

struct DemoState
{
    ThreadContext thread = {};
    Servers servers = {};
    AssetFactory assets = {};
    RenderingCompositor compositor = {};
    RenderAssetCache render_assets = {};
    GPUTransferQueue transfers = {};
    SDL_Window* window = nullptr;
    AssetHandle<MeshAsset> meshes[SDL_arraysize(demo_mesh_names)] = {};
    GeometryDraw draws[SDL_arraysize(demo_mesh_names)] = {};
    Mat4 nodes[SDL_arraysize(demo_mesh_names)] = {};
    b3Vec3 center = {};
    float scale = 1;
    Mat4 view = {};
    Mat4 camera = {};
    uint64_t refresh_at = 0;
    uint64_t started = 0;
};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!initialize_memory() || !SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Demo initialization failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    DemoState* demo = static_cast<DemoState*>(SDL_malloc(sizeof(DemoState)));
    std::construct_at(demo);
    *appstate = demo;
    if (!set_thread_context(&demo->thread) || !create_servers(demo->servers, {.thread_count = 2})) {
        SDL_Log("Demo servers failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    const char* base = SDL_GetBasePath();
    if (!base) {
        SDL_Log("Demo path failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    ArenaTemp scratch = scratch_begin();
    bool created = create_asset_factory(demo->assets, demo->servers.artifacts,
        argc > 1 ? argv[1] : path_join(*scratch.arena, base, "rendering_demo_assets"));
    scratch_end(scratch);
    if (!created) {
        SDL_Log("Demo assets failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    demo->window = SDL_CreateWindow("ORbit asset-to-GPU demo", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!demo->window) {
        SDL_Log("Demo window failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!create_rendering_compositor(demo->compositor, demo->window, demo->servers.artifacts,
                                    {.shader_directory = argc > 2 ? argv[2] : nullptr})) {
        SDL_Log("Demo renderer failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    demo->render_assets = {.assets = &demo->assets, .device = demo->compositor.device};
    for (uint32_t i = 0; i < SDL_arraysize(demo_mesh_names); ++i) {
        char name[128];
        SDL_snprintf(name, sizeof(name), "ChronographWatch.glb#%s", demo_mesh_names[i]);
        demo->meshes[i] = asset_load(demo->assets, name);
        if (!asset_valid(demo->assets, demo->meshes[i])) {
            SDL_Log("Demo mesh request failed: %s", SDL_GetError());
            return SDL_APP_FAILURE;
        }
        AssetID id = asset_status(demo->assets, demo->meshes[i]).id;
        demo->meshes[i] = request_render_mesh(demo->render_assets, id);
        SDL_Log("%s: AssetID %llu", name, static_cast<unsigned long long>(id));
    }
    demo->view = make_view_matrix({.p = {.z = 2}, .q = b3Quat_identity});
    int width, height;
    if (!SDL_GetWindowSizeInPixels(demo->window, &width, &height)) {
        SDL_Log("Demo window size failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    demo->camera = mul(make_perspective(B3_PI / 3, float(width) / float(height), .1f, 100), demo->view);
    demo->started = SDL_GetTicks();
    demo->refresh_at = demo->started + 500;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    DemoState& demo = *static_cast<DemoState*>(appstate);
    if (event->type == SDL_EVENT_QUIT || (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE))
        return SDL_APP_SUCCESS;
    if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED && event->window.data1 > 0 && event->window.data2 > 0)
        demo.camera = mul(make_perspective(B3_PI / 3, float(event->window.data1) / float(event->window.data2), .1f, 100), demo.view);
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    DemoState& demo = *static_cast<DemoState*>(appstate);
    ArtifactEvent event;
    while (command_next_event(demo.servers.artifacts.commands, event)) {
        SDL_GPUGraphicsPipeline* previous = demo.compositor.gbuffer.pipeline;
        if (rendering_process_event(demo.compositor, event)) {
            if (demo.compositor.shader_failed && !demo.compositor.depth_pass.pipeline)
                return SDL_APP_FAILURE;
            if (previous != demo.compositor.gbuffer.pipeline)
                SDL_Log("Shader pipelines ready%s", previous ? " (reloaded)" : "");
        } else if (assets_process_event(demo.assets, event)) {
            for (AssetHandle<MeshAsset> mesh : demo.meshes) {
                if (asset_status(demo.assets, mesh).error != AssetError::None) {
                    SDL_Log("Mesh load failed: %s", demo.assets.slots[mesh.index].message);
                    if (!asset_get(demo.assets, mesh))
                        return SDL_APP_FAILURE;
                }
            }
            if (event.result.scene) {
                const ImportedScene& scene = *event.result.scene;
                b3AABB bounds = {.lowerBound = {.x = FLT_MAX, .y = FLT_MAX, .z = FLT_MAX},
                                 .upperBound = {.x = -FLT_MAX, .y = -FLT_MAX, .z = -FLT_MAX}};
                bool found = false;
                for (size_t n = 0; n < scene.nodes.size; ++n) {
                    const ImportedNode& node = scene.nodes.data[n];
                    const char* name = scene.meshes.data[node.mesh].name;
                    if (!name)
                        continue;
                    for (uint32_t i = 0; i < SDL_arraysize(demo_mesh_names); ++i) {
                        if (SDL_strcmp(name, demo_mesh_names[i]) != 0)
                            continue;
                        SDL_memcpy(demo.nodes[i].m, node.world, sizeof(node.world));
                        const b3AABB& local = event.result.meshes.data[node.mesh].bounds;
                        for (uint32_t corner = 0; corner < 8; ++corner) {
                            b3Vec3 p = {.x = corner & 1 ? local.upperBound.x : local.lowerBound.x,
                                        .y = corner & 2 ? local.upperBound.y : local.lowerBound.y,
                                        .z = corner & 4 ? local.upperBound.z : local.lowerBound.z};
                            b3Vec3 world = {.x = node.world[0] * p.x + node.world[4] * p.y + node.world[8] * p.z + node.world[12],
                                            .y = node.world[1] * p.x + node.world[5] * p.y + node.world[9] * p.z + node.world[13],
                                            .z = node.world[2] * p.x + node.world[6] * p.y + node.world[10] * p.z + node.world[14]};
                            bounds.lowerBound = b3Min(bounds.lowerBound, world);
                            bounds.upperBound = b3Max(bounds.upperBound, world);
                        }
                        found = true;
                        break;
                    }
                }
                if (found) {
                    demo.center = .5f * (bounds.lowerBound + bounds.upperBound);
                    float radius = b3Length(bounds.upperBound - demo.center);
                    demo.scale = radius > 0 ? .8f / radius : 1;
                }
            }
        }
    }
    render_assets_process_changes(demo.render_assets);
    uint64_t now = SDL_GetTicks();
    if (now >= demo.refresh_at) {
        reload_rendering_shaders(demo.compositor);
        for (AssetHandle<MeshAsset> mesh : demo.meshes) {
            if (!asset_status(demo.assets, mesh).loading)
                asset_reload(demo.assets, mesh);
        }
        demo.refresh_at = now + 500;
    }
    assets_tick(demo.assets);
    rendering_shaders_tick(demo.compositor);
    for (uint32_t i = 0; i < demo.render_assets.pending_count; ++i) {
        const MeshAsset& mesh = *asset_get(demo.assets, demo.render_assets.entries[demo.render_assets.pending[i]].asset);
        uint32_t vertices = 0;
        for (size_t i = 0; i < mesh.primitives.size; ++i)
            vertices += mesh.primitives.data[i].vertex_count;
        SDL_Log("Mesh loaded: %u vertices, %zu indices; GPU upload queued", vertices,
                mesh.indices.size / (mesh.index_type == MeshIndexType::UInt16 ? 2 : 4));
    }
    prepare_render_assets(demo.render_assets, demo.transfers);
    bool uploading = demo.transfers.count != 0;
    b3Quat rotation = b3MakeQuatFromAxisAngle({.y = 1}, float((now - demo.started) % 20000) * (2 * B3_PI / 20000));
    Mat4 world = make_transform_matrix({.p = b3RotateVector(rotation, -demo.scale * demo.center), .q = rotation},
                                      {.x = demo.scale, .y = demo.scale, .z = demo.scale});
    for (uint32_t i = 0; i < SDL_arraysize(demo_mesh_names); ++i) {
        demo.draws[i].world = mul(world, demo.nodes[i]);
        demo.draws[i].mesh = render_mesh(demo.render_assets, demo.meshes[i]);
    }
    bool rendered = demo.compositor.render(demo.transfers, {demo.draws, SDL_arraysize(demo.draws)}, demo.camera);
    finish_render_assets(demo.render_assets, demo.transfers);
    if (!rendered) {
        SDL_Log("Demo rendering failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (uploading)
        SDL_Log("GPU mesh ready for the next frame");
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult)
{
    DemoState* demo = static_cast<DemoState*>(appstate);
    if (demo) {
        if (demo->compositor.device) {
            SDL_WaitForGPUIdle(demo->compositor.device);
            destroy_render_asset_cache(demo->render_assets);
            destroy_gpu_transfer_queue(demo->transfers, demo->compositor.device);
        }
        destroy_rendering_compositor(demo->compositor);
        destroy_asset_factory(demo->assets);
        destroy_servers(demo->servers);
        SDL_DestroyWindow(demo->window);
        destroy_thread_context(demo->thread);
        std::destroy_at(demo);
        SDL_free(demo);
    }
    SDL_Quit();
}
