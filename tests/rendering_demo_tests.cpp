#define SDL_MAIN_HANDLED
#define SDL_MAIN_NOIMPL
#include "../examples/rendering_demo/main.cpp"
#undef main
#include "mesh_fixture.h"
#include <rpmalloc.h>

#define CHECK(x) do { if (!(x)) { SDL_Log("%s:%d: %s (%s)", __FILE__, __LINE__, #x, SDL_GetError()); return false; } } while (0)

static bool iterate(DemoState& demo)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
        CHECK(SDL_AppEvent(&demo, &event) == SDL_APP_CONTINUE);
    CHECK(SDL_AppIterate(&demo) == SDL_APP_CONTINUE);
    return true;
}

static bool capture(DemoState& demo)
{
    uint32_t width = demo.compositor.width, height = demo.compositor.height;
    uint32_t pitch = (width * 4 + 255) / 256 * 256;
    SDL_GPUTransferBufferCreateInfo info = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, .size = pitch * height};
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(demo.compositor.device, &info);
    CHECK(transfer);
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(demo.compositor.device);
    CHECK(commands);
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUTextureRegion from = {.texture = demo.compositor.colors[0], .w = width, .h = height, .d = 1};
    SDL_GPUTextureTransferInfo to = {.transfer_buffer = transfer, .pixels_per_row = pitch / 4, .rows_per_layer = height};
    SDL_DownloadFromGPUTexture(copy, &from, &to);
    SDL_EndGPUCopyPass(copy);
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
    CHECK(fence && SDL_WaitForGPUFences(demo.compositor.device, true, &fence, 1));
    uint8_t* pixels = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(demo.compositor.device, transfer, false));
    CHECK(pixels);
    uint32_t visible = 0;
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x)
            visible += pixels[size_t(y) * pitch + x * 4 + 2] >= 50;
    }
    CHECK(pixels[2] <= 6 && visible > width * height / 100);
    SDL_Surface* surface = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, pixels, pitch);
    CHECK(surface && SDL_SaveBMP(surface, ORBIT_TEST_DIRECTORY "/rendering_demo.bmp"));
    SDL_DestroySurface(surface);
    SDL_UnmapGPUTransferBuffer(demo.compositor.device, transfer);
    SDL_ReleaseGPUFence(demo.compositor.device, fence);
    SDL_ReleaseGPUTransferBuffer(demo.compositor.device, transfer);
    return true;
}

static bool test_demo(DemoState& demo)
{
    CHECK(!render_mesh(demo.render_assets, demo.meshes[0]));
    CHECK(asset_status(demo.assets, demo.meshes[0]).id == asset_id("ChronographWatch.glb#Backplate Khronos"));
    bool ready = false;
    for (uint32_t i = 0; i < 1000 && !ready; ++i) {
        CHECK(iterate(demo));
        ready = demo.compositor.width != 0;
        for (const GeometryDraw& draw : demo.draws)
            ready = ready && draw.mesh;
    }
    CHECK(ready && demo.assets.count == SDL_arraysize(demo_mesh_names));
    SDL_GPUBuffer* mesh_vertices[SDL_arraysize(demo_mesh_names)];
    for (uint32_t i = 0; i < SDL_arraysize(demo_mesh_names); ++i) {
        CHECK(demo.draws[i].mesh->primitives.size == asset_get(demo.assets, demo.meshes[i])->primitives.size);
        mesh_vertices[i] = demo.draws[i].mesh->vertices;
    }
    CHECK(SDL_fabsf(demo.nodes[0].m[6] + 1) < .001f && SDL_fabsf(demo.nodes[0].m[14] - .01f) < .001f);
    CHECK(demo.scale > 0 && demo.scale < .2f);
    CHECK(demo.draws[0].mesh && demo.compositor.depth_pass.pipeline && demo.compositor.gbuffer.pipeline);
    CHECK(demo.render_assets.entries[demo.meshes[0].index].state == GPUAssetState::Ready);
    CHECK(demo.transfers.staging && demo.draws[0].mesh->primitives.size == 1);
    CHECK(capture(demo));
    SDL_GPUBuffer* vertices = demo.draws[0].mesh->vertices;
    SDL_GPUTransferBuffer* staging = demo.transfers.staging;
    SDL_GPUGraphicsPipeline* depth = demo.compositor.depth_pass.pipeline;
    SDL_GPUGraphicsPipeline* gbuffer = demo.compositor.gbuffer.pipeline;
    demo.refresh_at = UINT64_MAX;
    rpmalloc_thread_statistics_t before, after;
    rpmalloc_thread_statistics(&before);
    for (uint32_t i = 0; i < 60; ++i) {
        CHECK(iterate(demo));
        for (uint32_t m = 0; m < SDL_arraysize(demo_mesh_names); ++m)
            CHECK(demo.draws[m].mesh->vertices == mesh_vertices[m]);
        CHECK(demo.transfers.staging == staging);
        CHECK(!demo.transfers.count && !demo.render_assets.pending_count);
        CHECK(demo.compositor.depth_pass.pipeline == depth && demo.compositor.gbuffer.pipeline == gbuffer);
    }
    rpmalloc_thread_statistics(&after);
    size_t allocations = 0;
    for (uint32_t i = 0; i < 128; ++i)
        allocations += after.size_use[i].alloc_total - before.size_use[i].alloc_total;
    SDL_Log("Demo: 60 steady frames, %zu allocator calls including SDL/backend; mesh/staging/pipelines unchanged", allocations);

    const char* extension = demo.compositor.shader_format == SDL_GPU_SHADERFORMAT_MSL ? "msl" :
        (demo.compositor.shader_format == SDL_GPU_SHADERFORMAT_DXIL ? "dxil" : "spv");
    char replacement_shader[1024];
    SDL_snprintf(replacement_shader, sizeof(replacement_shader), "%s/gbuffer_reload.frag.%s", ORBIT_TEST_DIRECTORY, extension);
    CHECK(SDL_CopyFile(replacement_shader, demo.compositor.shaders[2].path));
    demo.refresh_at = 0;
    for (uint32_t i = 0; i < 1000 && demo.compositor.gbuffer.pipeline == gbuffer; ++i)
        CHECK(iterate(demo));
    CHECK(demo.compositor.gbuffer.pipeline != gbuffer && demo.compositor.depth_pass.pipeline == depth);
    CHECK(!demo.compositor.shader_failed && demo.draws[0].mesh->vertices == vertices);
    SDL_Log("Demo: shader hot reload verified");

    size_t source_size = 0;
    uint8_t* source = static_cast<uint8_t*>(SDL_LoadFile(ORBIT_DEMO_ASSET, &source_size));
    CHECK(source);
    uint32_t json_size;
    SDL_memcpy(&json_size, source + 12, sizeof(json_size));
    json_size = SDL_Swap32LE(json_size);
    uint8_t* changed = static_cast<uint8_t*>(SDL_malloc(source_size + 4));
    SDL_memcpy(changed, source, 20 + json_size);
    SDL_memset(changed + 20 + json_size, ' ', 4);
    SDL_memcpy(changed + 24 + json_size, source + 20 + json_size, source_size - 20 - json_size);
    put_u32(changed + 8, uint32_t(source_size + 4));
    put_u32(changed + 12, json_size + 4);
    CHECK(SDL_SaveFile(demo.assets.slots[demo.meshes[0].index].path, changed, source_size + 4));
    SDL_free(changed);
    SDL_free(source);
    demo.refresh_at = 0;
    bool reloaded = false;
    for (uint32_t i = 0; i < 1000 && !reloaded; ++i) {
        CHECK(iterate(demo));
        reloaded = true;
        for (uint32_t m = 0; m < SDL_arraysize(demo_mesh_names); ++m)
            reloaded = reloaded && render_mesh(demo.render_assets, demo.meshes[m])->vertices != mesh_vertices[m];
    }
    CHECK(reloaded);
    CHECK(render_mesh(demo.render_assets, demo.meshes[0])->vertices != vertices);
    CHECK(demo.transfers.staging == staging && asset_status(demo.assets, demo.meshes[0]).generation == 2);
    CHECK(iterate(demo) && demo.draws[0].mesh == render_mesh(demo.render_assets, demo.meshes[0]));
    SDL_Log("Demo: mesh hot reload verified");

    const int sizes[][2] = {{640, 480}, {1024, 400}, {800, 600}};
    for (const int (&size)[2] : sizes) {
        uint32_t old_width = demo.compositor.width, old_height = demo.compositor.height;
        CHECK(SDL_SetWindowSize(demo.window, size[0], size[1]));
        bool resized = false;
        for (uint32_t i = 0; i < 100 && !resized; ++i) {
            CHECK(iterate(demo));
            int width, height;
            CHECK(SDL_GetWindowSizeInPixels(demo.window, &width, &height));
            resized = (demo.compositor.width != old_width || demo.compositor.height != old_height) &&
                demo.compositor.width == uint32_t(width) && demo.compositor.height == uint32_t(height);
            if (!resized) SDL_Delay(10);
        }
        CHECK(resized && demo.compositor.depth_pass.pipeline == depth);
        CHECK(SDL_fabsf(demo.camera.m[0] * float(demo.compositor.width) / demo.compositor.height - demo.camera.m[5]) < .001f);
    }
    SDL_Log("Demo: repeated resize verified");
    SDL_Event quit = {.type = SDL_EVENT_QUIT};
    CHECK(SDL_AppEvent(&demo, &quit) == SDL_APP_SUCCESS);
    return true;
}

int main()
{
    SDL_SetMainReady();
    if (!initialize_memory() || !SDL_CreateDirectory(ORBIT_TEST_DIRECTORY "/demo-assets") || !SDL_CreateDirectory(ORBIT_TEST_DIRECTORY "/demo-shaders"))
        return 1;
    if (!SDL_CopyFile(ORBIT_DEMO_ASSET, ORBIT_TEST_DIRECTORY "/demo-assets/ChronographWatch.glb"))
        return 1;
    const char* names[] = {"geometry.vert", "depth.frag", "gbuffer.frag"};
#if defined(__APPLE__)
    const char* extension = "msl";
#elif defined(_WIN32)
    const char* extension = "dxil";
#else
    const char* extension = "spv";
#endif
    for (const char* name : names) {
        char from[1024], to[1024];
        SDL_snprintf(from, sizeof(from), "%s/%s.%s", ORBIT_SHADER_DIRECTORY, name, extension);
        SDL_snprintf(to, sizeof(to), "%s/demo-shaders/%s.%s", ORBIT_TEST_DIRECTORY, name, extension);
        if (!SDL_CopyFile(from, to))
            return 1;
    }
    char name[] = "rendering_demo_tests", assets[] = ORBIT_TEST_DIRECTORY "/demo-assets", shaders[] = ORBIT_TEST_DIRECTORY "/demo-shaders";
    char* argv[] = {name, assets, shaders};
    void* appstate = nullptr;
    bool success = SDL_AppInit(&appstate, 3, argv) == SDL_APP_CONTINUE && test_demo(*static_cast<DemoState*>(appstate));
    SDL_AppQuit(appstate, success ? SDL_APP_SUCCESS : SDL_APP_FAILURE);
    return success ? 0 : 1;
}
