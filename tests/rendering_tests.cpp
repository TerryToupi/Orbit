#include <render/rendering_compositor.h>
#include <render/render_asset_cache.h>
#include "mesh_fixture.h"
#include <memory_setup.h>
#include <thread_context.h>
#include <servers.h>
#include <rpmalloc.h>
#include <SDL3/SDL.h>

#define CHECK(x) do { if (!(x)) { SDL_Log("%s:%d: %s (%s)", __FILE__, __LINE__, #x, SDL_GetError()); return false; } } while (0)

static bool render_frame(RenderingCompositor& compositor, Span<GeometryDraw> draws, const Mat4& camera)
{
    GPUTransferQueue transfers;
    CHECK(compositor.render(transfers, draws, camera));
    CHECK(!transfers.submitted);
    return true;
}

static bool upload_mesh(RenderingCompositor& compositor, GPUTransferQueue& transfers, const MeshAsset& source, GpuMesh& mesh)
{
    CHECK(create_gpu_mesh(compositor.device, source, mesh));
    CHECK(begin_gpu_uploads(transfers, compositor.device, mesh.vertex_bytes + source.indices.size, 2));
    uint8_t* vertices = queue_gpu_upload(transfers, mesh.vertices, mesh.vertex_bytes);
    uint8_t* indices = queue_gpu_upload(transfers, mesh.indices, source.indices.size);
    CHECK(pack_gpu_mesh(source, mesh, vertices, indices));
    CHECK(compositor.render(transfers, {}, {}));
    reset_gpu_transfers(transfers, compositor.device);
    return true;
}

struct TestScene
{
    GeometryDraw instances[2] = {};
    size_t count = 1;
    Mat4 camera = {};
};

static bool finish_shaders(RenderingCompositor& compositor)
{
    for (uint32_t i = 0; i < 1000; ++i) {
        ArtifactEvent event;
        while (command_next_event(compositor.artifacts->commands, event))
            CHECK(rendering_process_event(compositor, event));
        rendering_shaders_tick(compositor);
        if (!compositor.shader_pending && !compositor.shader_queued)
            return true;
        SDL_Delay(1);
    }
    return false;
}

static bool read_pixel(RenderingCompositor& compositor, uint32_t target, uint32_t x, uint32_t y, uint8_t (&pixel)[4])
{
    uint32_t row = (compositor.width * 4 + 255) / 256 * 256;
    SDL_GPUTransferBufferCreateInfo info = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, .size = row * compositor.height};
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(compositor.device, &info);
    CHECK(transfer);
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(compositor.device);
    CHECK(commands);
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUTextureRegion from = {.texture = compositor.colors[target], .w = compositor.width, .h = compositor.height, .d = 1};
    SDL_GPUTextureTransferInfo to = {.transfer_buffer = transfer, .pixels_per_row = row / 4, .rows_per_layer = compositor.height};
    SDL_DownloadFromGPUTexture(copy, &from, &to);
    SDL_EndGPUCopyPass(copy);
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
    CHECK(fence && SDL_WaitForGPUFences(compositor.device, true, &fence, 1));
    const uint8_t* mapped = static_cast<const uint8_t*>(SDL_MapGPUTransferBuffer(compositor.device, transfer, false));
    CHECK(mapped);
    SDL_memcpy(pixel, mapped + size_t(y) * row + x * 4, 4);
    SDL_UnmapGPUTransferBuffer(compositor.device, transfer);
    SDL_ReleaseGPUFence(compositor.device, fence);
    SDL_ReleaseGPUTransferBuffer(compositor.device, transfer);
    return true;
}

static bool render_visible(RenderingCompositor& compositor, const TestScene& scene)
{
    for (uint32_t i = 0; i < 100; ++i) {
        SDL_PumpEvents();
        CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
        if (compositor.width)
            return true;
        SDL_Delay(10);
    }
    return false;
}

static bool copy_shader(const char* directory, const char* name, const char* destination, const char* extension)
{
    char from[1024], to[1024];
    SDL_snprintf(from, sizeof(from), "%s/%s.%s", directory, name, extension);
    SDL_snprintf(to, sizeof(to), "%s/reload-shaders/%s.%s", ORBIT_TEST_DIRECTORY, destination, extension);
    CHECK(SDL_CopyFile(from, to));
    return true;
}

static bool test_reload(RenderingCompositor& compositor, const TestScene& scene)
{
    const char* extension = compositor.shader_format == SDL_GPU_SHADERFORMAT_MSL ? "msl" :
                           (compositor.shader_format == SDL_GPU_SHADERFORMAT_DXIL ? "dxil" : "spv");
    SDL_GPUGraphicsPipeline* depth = compositor.depth_pass.pipeline;
    SDL_GPUGraphicsPipeline* gbuffer = compositor.gbuffer.pipeline;
    SDL_GPUShader* vertex = compositor.shaders[0].shader;
    reload_rendering_shaders(compositor);
    CHECK(finish_shaders(compositor) && !compositor.shader_failed);
    CHECK(depth == compositor.depth_pass.pipeline && gbuffer == compositor.gbuffer.pipeline && vertex == compositor.shaders[0].shader);

    CHECK(copy_shader(ORBIT_TEST_DIRECTORY, "gbuffer_reload.frag", "gbuffer.frag", extension));
    reload_rendering_shaders(compositor);
    CHECK(finish_shaders(compositor) && !compositor.shader_failed);
    CHECK(depth == compositor.depth_pass.pipeline && gbuffer != compositor.gbuffer.pipeline && vertex == compositor.shaders[0].shader);
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    uint8_t pixel[4];
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel));
    CHECK(pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 0);
    gbuffer = compositor.gbuffer.pipeline;

    CHECK(copy_shader(ORBIT_TEST_DIRECTORY, "geometry_reload.vert", "geometry.vert", extension));
    reload_rendering_shaders(compositor);
    CHECK(finish_shaders(compositor) && !compositor.shader_failed);
    CHECK(depth != compositor.depth_pass.pipeline && gbuffer != compositor.gbuffer.pipeline && vertex != compositor.shaders[0].shader);
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[0] == 255);
    gbuffer = compositor.gbuffer.pipeline;
    depth = compositor.depth_pass.pipeline;
    vertex = compositor.shaders[0].shader;

    CHECK(copy_shader(ORBIT_TEST_DIRECTORY, "depth_reload.frag", "depth.frag", extension));
    reload_rendering_shaders(compositor);
    CHECK(finish_shaders(compositor) && !compositor.shader_failed);
    CHECK(depth != compositor.depth_pass.pipeline && gbuffer == compositor.gbuffer.pipeline);
    depth = compositor.depth_pass.pipeline;

    const char* fragment_path = compositor.shaders[2].path;
    CHECK(SDL_RemovePath(fragment_path));
    reload_rendering_shaders(compositor);
    CHECK(finish_shaders(compositor) && compositor.shader_failed);
    CHECK(depth == compositor.depth_pass.pipeline && gbuffer == compositor.gbuffer.pipeline);
    CHECK(SDL_SaveFile(fragment_path, "", 0));
    reload_rendering_shaders(compositor);
    CHECK(finish_shaders(compositor) && compositor.shader_failed);
    CHECK(depth == compositor.depth_pass.pipeline && gbuffer == compositor.gbuffer.pipeline);
    if (compositor.shader_format == SDL_GPU_SHADERFORMAT_MSL) {
        CHECK(SDL_SaveFile(fragment_path, "invalid shader", 14));
        reload_rendering_shaders(compositor);
        CHECK(finish_shaders(compositor) && compositor.shader_failed);
        CHECK(depth == compositor.depth_pass.pipeline && gbuffer == compositor.gbuffer.pipeline);
        reload_rendering_shaders(compositor);
        CHECK(finish_shaders(compositor) && compositor.shader_failed);
        CHECK(depth == compositor.depth_pass.pipeline && gbuffer == compositor.gbuffer.pipeline);
        // Both pipelines would change; failure of the second must roll back the first as well.
        CHECK(copy_shader(ORBIT_SHADER_DIRECTORY, "geometry.vert", "geometry.vert", extension));
        CHECK(copy_shader(ORBIT_TEST_DIRECTORY, "invalid_pipeline.frag", "gbuffer.frag", extension));
        reload_rendering_shaders(compositor);
        CHECK(finish_shaders(compositor) && compositor.shader_failed);
        CHECK(depth == compositor.depth_pass.pipeline && gbuffer == compositor.gbuffer.pipeline && vertex == compositor.shaders[0].shader);
    }
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[0] == 255 && pixel[2] == 0);

    CHECK(copy_shader(ORBIT_SHADER_DIRECTORY, "geometry.vert", "geometry.vert", extension));
    CHECK(copy_shader(ORBIT_SHADER_DIRECTORY, "depth.frag", "depth.frag", extension));
    CHECK(copy_shader(ORBIT_SHADER_DIRECTORY, "gbuffer.frag", "gbuffer.frag", extension));
    reload_rendering_shaders(compositor);
    CHECK(finish_shaders(compositor) && !compositor.shader_failed);
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] >= 203);
    return true;
}

static bool finish_assets(AssetFactory& factory, RenderAssetCache& cache)
{
    for (uint32_t i = 0; i < 1000; ++i) {
        ArtifactEvent event;
        while (command_next_event(factory.artifacts->commands, event))
            CHECK(assets_process_event(factory, event));
        render_assets_process_changes(cache);
        assets_tick(factory);
        bool pending = false;
        for (uint32_t j = 0; j < factory.count; ++j)
            pending |= factory.slots[j].phase != AssetPhase::None;
        if (!pending)
            return true;
        SDL_Delay(1);
    }
    return false;
}

static bool read_buffer(SDL_GPUDevice* device, SDL_GPUBuffer* buffer, uint32_t offset, void* bytes, uint32_t size)
{
    SDL_GPUTransferBufferCreateInfo info = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, .size = size};
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &info);
    CHECK(transfer);
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(device);
    CHECK(commands);
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUBufferRegion from = {.buffer = buffer, .offset = offset, .size = size};
    SDL_GPUTransferBufferLocation to = {.transfer_buffer = transfer};
    SDL_DownloadFromGPUBuffer(copy, &from, &to);
    SDL_EndGPUCopyPass(copy);
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
    CHECK(fence && SDL_WaitForGPUFences(device, true, &fence, 1));
    const void* mapped = SDL_MapGPUTransferBuffer(device, transfer, false);
    CHECK(mapped);
    SDL_memcpy(bytes, mapped, size);
    SDL_UnmapGPUTransferBuffer(device, transfer);
    SDL_ReleaseGPUFence(device, fence);
    SDL_ReleaseGPUTransferBuffer(device, transfer);
    return true;
}

static bool test_render_assets(RenderingCompositor& compositor, Servers& servers)
{
    Arena fixtures;
    Span<uint8_t> original = fixture(fixtures, "triangle.json");
    Span<uint8_t> replacement = fixture(fixtures, "triangle.json", 1);
    CHECK(original.size && replacement.size);
    CHECK(SDL_SaveFile(ORBIT_TEST_DIRECTORY "/gpu-a.glb", original.data, original.size));
    CHECK(SDL_SaveFile(ORBIT_TEST_DIRECTORY "/gpu-b.glb", replacement.data, replacement.size));
    AssetFactory assets;
    CHECK(create_asset_factory(assets, servers.artifacts, ORBIT_TEST_DIRECTORY));
    RenderAssetCache cache = {.assets = &assets, .device = compositor.device};
    GPUTransferQueue transfers;
    AssetHandle<MeshAsset> a = asset_load(assets, "gpu-a.glb");
    AssetHandle<MeshAsset> b = asset_load(assets, "gpu-b.glb");
    CHECK(request_render_mesh(cache, asset_id("gpu-a.glb")) == a);
    CHECK(request_render_mesh(cache, asset_id("gpu-b.glb")) == b);
    CHECK(finish_assets(assets, cache));
    CHECK(cache.pending_count == 2 && !render_mesh(cache, a) && !render_mesh(cache, b));
    CHECK(request_render_mesh(cache, asset_id("gpu-a.glb")) == a && cache.pending_count == 2);
    prepare_render_assets(cache, transfers);
    CHECK(transfers.count == 4 && transfers.mapped && transfers.staging);
    CHECK(!render_mesh(cache, a) && cache.entries[a.index].state == GPUAssetState::PendingUpload);
    CHECK(transfers.uploads[2].offset % 4 == 0);
    CHECK(compositor.render(transfers, {}, {}));
    CHECK(transfers.submitted && !transfers.mapped && !render_mesh(cache, a));
    finish_render_assets(cache, transfers);
    CHECK(cache.entries[a.index].state == GPUAssetState::Ready && render_mesh(cache, a) && render_mesh(cache, b));
    CHECK(!cache.pending_count && !transfers.count);
    float vertex[6];
    CHECK(read_buffer(compositor.device, render_mesh(cache, a)->vertices, 24, vertex, sizeof(vertex)) && vertex[0] == 1 && vertex[5] == 1);
    CHECK(read_buffer(compositor.device, render_mesh(cache, b)->vertices, 24, vertex, sizeof(vertex)) && vertex[0] == 2 && vertex[5] == 1);
    uint16_t indices[3];
    CHECK(read_buffer(compositor.device, render_mesh(cache, b)->indices, 0, indices, sizeof(indices)) && indices[1] == 1 && indices[2] == 2);
    GeometryDraw draw = {.mesh = render_mesh(cache, a), .world = make_transform_matrix({.p = {.x = -.25f, .y = -.25f}, .q = b3Quat_identity})};
    CHECK(compositor.render(transfers, {&draw, 1}, {}));
    finish_render_assets(cache, transfers);
    uint8_t pixel[4];
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] >= 203);
    SDL_GPUBuffer* live = render_mesh(cache, a)->vertices;
    SDL_GPUTransferBuffer* staging = transfers.staging;
    size_t capacity = transfers.capacity;
    rpmalloc_thread_statistics_t before, after;
    rpmalloc_thread_statistics(&before);
    for (uint32_t i = 0; i < 60; ++i) {
        render_assets_process_changes(cache);
        prepare_render_assets(cache, transfers);
        CHECK(render_mesh(cache, a)->vertices == live && !transfers.count && !transfers.mapped);
        finish_render_assets(cache, transfers);
    }
    rpmalloc_thread_statistics(&after);
    for (uint32_t i = 0; i < 128; ++i)
        CHECK(after.size_use[i].alloc_total == before.size_use[i].alloc_total);
    SDL_Log("60 unchanged asset preparation cycles: zero allocator calls");
    asset_reload(assets, a);
    CHECK(finish_assets(assets, cache));
    CHECK(!cache.pending_count && render_mesh(cache, a)->vertices == live);
    prepare_render_assets(cache, transfers);
    CHECK(!transfers.mapped && !transfers.count && transfers.staging == staging);
    CHECK(compositor.render(transfers, {&draw, 1}, {}));
    finish_render_assets(cache, transfers);

    CHECK(SDL_SaveFile(ORBIT_TEST_DIRECTORY "/gpu-a.glb", replacement.data, replacement.size));
    asset_reload(assets, a);
    CHECK(finish_assets(assets, cache));
    CHECK(cache.pending_count == 1 && render_mesh(cache, a)->vertices == live);
    prepare_render_assets(cache, transfers);
    CHECK(transfers.count == 2 && transfers.staging == staging && transfers.capacity == capacity);
    CHECK(render_mesh(cache, a)->vertices == live);
    // Abandoned transfer work must preserve the previously drawable version.
    SDL_GPUCommandBuffer* abandoned = SDL_AcquireGPUCommandBuffer(compositor.device);
    CHECK(abandoned);
    encode_gpu_transfers(transfers, compositor.device, abandoned);
    CHECK(SDL_CancelGPUCommandBuffer(abandoned));
    finish_render_assets(cache, transfers);
    CHECK(render_mesh(cache, a)->vertices == live && cache.entries[a.index].state == GPUAssetState::Ready);
    CHECK(request_render_mesh(cache, asset_id("gpu-a.glb")) == a && !cache.pending_count);

    Span<uint8_t> fresh = fixture(fixtures, "triangle.json", 2);
    CHECK(SDL_SaveFile(ORBIT_TEST_DIRECTORY "/gpu-a.glb", fresh.data, fresh.size));
    asset_reload(assets, a);
    CHECK(finish_assets(assets, cache));
    prepare_render_assets(cache, transfers);
    draw.mesh = render_mesh(cache, a);
    CHECK(compositor.render(transfers, {&draw, 1}, {}));
    CHECK(render_mesh(cache, a)->vertices == live);
    finish_render_assets(cache, transfers);
    CHECK(render_mesh(cache, a)->vertices != live);
    CHECK(read_buffer(compositor.device, render_mesh(cache, a)->vertices, 24, vertex, sizeof(vertex)) && vertex[0] == 3);
    live = render_mesh(cache, a)->vertices;
    CHECK(SDL_SaveFile(ORBIT_TEST_DIRECTORY "/gpu-a.glb", "bad", 3));
    asset_reload(assets, a);
    CHECK(finish_assets(assets, cache));
    CHECK(!cache.pending_count && render_mesh(cache, a)->vertices == live);

    // Grow staging while its previous submissions may still be in flight, then cycle the larger buffer.
    SDL_GPUBufferCreateInfo buffer_info = {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = uint32_t(capacity + 4)};
    SDL_GPUBuffer* buffer = SDL_CreateGPUBuffer(compositor.device, &buffer_info);
    CHECK(buffer);
    CHECK(begin_gpu_uploads(transfers, compositor.device, 4, 1));
    SDL_memset(queue_gpu_upload(transfers, buffer, 4), 7, 4);
    CHECK(compositor.render(transfers, {}, {}));
    reset_gpu_transfers(transfers, compositor.device);
    CHECK(begin_gpu_uploads(transfers, compositor.device, buffer_info.size, 1));
    CHECK(transfers.capacity > capacity && transfers.staging != staging);
    SDL_memset(queue_gpu_upload(transfers, buffer, buffer_info.size), 19, buffer_info.size);
    CHECK(compositor.render(transfers, {}, {}));
    reset_gpu_transfers(transfers, compositor.device);
    staging = transfers.staging;
    capacity = transfers.capacity;
    CHECK(begin_gpu_uploads(transfers, compositor.device, 4, 1));
    SDL_memset(queue_gpu_upload(transfers, buffer, 4), 37, 4);
    CHECK(compositor.render(transfers, {}, {}));
    reset_gpu_transfers(transfers, compositor.device);
    CHECK(transfers.capacity == capacity && transfers.staging == staging);
    CHECK(read_buffer(compositor.device, buffer, 0, pixel, 4) && pixel[0] == 37);
    CHECK(read_buffer(compositor.device, buffer, buffer_info.size - 4, pixel, 4) && pixel[0] == 19);
    SDL_ReleaseGPUBuffer(compositor.device, buffer);

    asset_unload(assets, a);
    asset_unload(assets, b);
    render_assets_process_changes(cache);
    CHECK(!render_mesh(cache, a) && !render_mesh(cache, b));
    CHECK(!cache.entries[a.index].mesh.vertices && !cache.entries[b.index].mesh.indices);
    CHECK(SDL_WaitForGPUIdle(compositor.device));
    destroy_render_asset_cache(cache);
    destroy_gpu_transfer_queue(transfers, compositor.device);
    destroy_asset_factory(assets);
    destroy_arena(fixtures);
    CHECK(SDL_RemovePath(ORBIT_TEST_DIRECTORY "/gpu-a.glb") && SDL_RemovePath(ORBIT_TEST_DIRECTORY "/gpu-b.glb"));
    return true;
}

static bool test_rendering(SDL_Window* window, Servers& servers)
{
    CHECK(SDL_CreateDirectory(ORBIT_TEST_DIRECTORY "/reload-shaders"));
    const char* extensions[] = {"spv",
#if defined(__APPLE__)
        "msl"
#elif defined(_WIN32)
        "dxil"
#endif
    };
    for (const char* extension : extensions) {
        CHECK(copy_shader(ORBIT_SHADER_DIRECTORY, "geometry.vert", "geometry.vert", extension));
        CHECK(copy_shader(ORBIT_SHADER_DIRECTORY, "depth.frag", "depth.frag", extension));
        CHECK(copy_shader(ORBIT_SHADER_DIRECTORY, "gbuffer.frag", "gbuffer.frag", extension));
    }
    RenderingCompositor compositor;
    CHECK(create_rendering_compositor(compositor, window, servers.artifacts, {.shader_directory = "/nonexistent/orbit-shaders"}));
    CHECK(finish_shaders(compositor) && compositor.shader_failed && !compositor.depth_pass.pipeline);
    destroy_rendering_compositor(compositor);
    CHECK(!compositor.device && !compositor.window);
    CHECK(create_rendering_compositor(compositor, window, servers.artifacts, {.shader_directory = ORBIT_TEST_DIRECTORY "/reload-shaders"}));
    CHECK(finish_shaders(compositor) && !compositor.shader_failed && compositor.depth_pass.pipeline && compositor.gbuffer.pipeline);
    SDL_Log("Rendering tests on SDL GPU driver: %s", SDL_GetGPUDeviceDriver(compositor.device));

    Arena storage;
    const float near_positions[] = {-.7f, -.7f, .2f, .7f, -.7f, .2f, 0, .7f, .2f};
    const float far_positions[] = {-.7f, -.7f, .8f, .7f, -.7f, .8f, 0, .7f, .8f};
    const float normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1};
    const float far_normals[] = {1, 0, 0, 1, 0, 0, 1, 0, 0};
    const uint32_t indices[] = {0, 1, 2};
    ImportedPrimitive primitives[] = {
        {.positions = near_positions, .normals = normals, .indices = indices},
        {.positions = far_positions, .normals = far_normals, .indices = indices}
    };
    MeshAsset mesh;
    char error[160] = {};
    CHECK(build_mesh_asset(storage, {.primitives = primitives}, mesh, error));
    TestScene scene;
    GpuMesh gpu_meshes[5] = {};
    GPUTransferQueue transfers;
    CHECK(upload_mesh(compositor, transfers, mesh, gpu_meshes[0]));
    scene.instances[0].mesh = &gpu_meshes[0];
    CHECK(render_visible(compositor, scene));
    CHECK(gpu_meshes[0].primitives.size == 2);
    uint8_t pixel[4];
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel));
    CHECK(pixel[0] >= 50 && pixel[0] <= 52 && pixel[2] >= 203 && pixel[3] == 255);
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel));
    CHECK(pixel[0] >= 127 && pixel[0] <= 128 && pixel[2] == 255 && pixel[3] >= 203);

    const GpuMesh& gpu = gpu_meshes[0];
    GpuMesh far_only = {.vertices = gpu.vertices, .indices = gpu.indices, .primitives = {gpu.primitives.data + 1, 1}, .index_type = gpu.index_type};
    GeometryDraw far_draw = {.mesh = &far_only, .world = scene.instances[0].world};
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(compositor.device);
    CHECK(commands);
    SDL_PushGPUVertexUniformData(commands, 0, &scene.camera, sizeof(scene.camera));
    gbuffer_pass(commands, compositor.gbuffer, compositor.depth, compositor.colors, {&far_draw, 1});
    CHECK(SDL_SubmitGPUCommandBuffer(commands));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel));
    CHECK(pixel[0] <= 6 && pixel[2] <= 6);

    scene.instances[0].world.m[12] = .9f;
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] <= 6);
    scene.camera.m[12] = -.9f;
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] >= 203);

    scene.instances[1] = scene.instances[0];
    scene.count = 2;
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    scene.count = 0;
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] <= 6);
    scene.count = 1;
    scene.instances[0].world.m[12] = 0;
    scene.camera.m[12] = 0;

    float diagonal[] = {.70710678f, .70710678f, 0, .70710678f, .70710678f, 0, .70710678f, .70710678f, 0};
    ImportedPrimitive transformed = {.positions = near_positions, .normals = diagonal, .indices = indices};
    MeshAsset replacement;
    CHECK(build_mesh_asset(storage, {.primitives = {&transformed, 1}}, replacement, error));
    CHECK(upload_mesh(compositor, transfers, replacement, gpu_meshes[1]));
    scene.instances[0].mesh = &gpu_meshes[1];
    scene.instances[0].world.m[0] = -2;
    scene.instances[0].world.m[5] = .5f;
    scene.camera.m[0] = -.5f;
    scene.camera.m[5] = 2;
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel));
    CHECK(pixel[0] >= 95 && pixel[0] <= 98 && pixel[1] >= 250 && pixel[2] >= 127 && pixel[2] <= 128);

    uint32_t old_width = compositor.width;
    SDL_GPUGraphicsPipeline* resize_pipeline = compositor.gbuffer.pipeline;
    CHECK(SDL_SetWindowSize(window, 192, 160));
    for (uint32_t i = 0; i < 100 && compositor.width == old_width; ++i) {
        SDL_PumpEvents();
        CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
        SDL_Delay(10);
    }
    CHECK(compositor.width != old_width);
    CHECK(compositor.gbuffer.pipeline == resize_pipeline);
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel) && pixel[1] >= 250);
    scene.instances[0].world.m[0] = 1;
    scene.instances[0].world.m[5] = 1;
    scene.camera.m[0] = 1;
    scene.camera.m[5] = 1;
    const uint32_t wide_indices[] = {0, 1, 2, 0, 1, 2};
    MeshAsset wide = mesh;
    wide.indices = {reinterpret_cast<const uint8_t*>(wide_indices), sizeof(wide_indices)};
    wide.index_type = MeshIndexType::UInt32;
    CHECK(upload_mesh(compositor, transfers, wide, gpu_meshes[2]));
    scene.instances[0].mesh = &gpu_meshes[2];
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(gpu_meshes[2].index_type == SDL_GPU_INDEXELEMENTSIZE_32BIT);
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] >= 203);

    ImportedPrimitive bare = {.positions = near_positions, .indices = indices};
    MeshAsset positions_only;
    CHECK(build_mesh_asset(storage, {.primitives = {&bare, 1}}, positions_only, error));
    CHECK(upload_mesh(compositor, transfers, positions_only, gpu_meshes[3]));
    scene.instances[0].mesh = &gpu_meshes[3];
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] == 255);

    float packed[18];
    for (uint32_t i = 0; i < 3; ++i) {
        SDL_memcpy(packed + i * 6, near_positions + i * 3, 12);
        SDL_memcpy(packed + i * 6 + 3, normals + i * 3, 12);
    }
    VertexAttribute attributes[] = {
        {.semantic = VertexSemantic::Position, .format = VertexFormat::Float3},
        {.semantic = VertexSemantic::Normal, .format = VertexFormat::Float3, .offset = 12}
    };
    VertexStream stream = {.data = {reinterpret_cast<const uint8_t*>(packed), sizeof(packed)}, .attributes = attributes, .stride = 24};
    MeshAsset interleaved = positions_only;
    interleaved.streams = {&stream, 1};
    CHECK(upload_mesh(compositor, transfers, interleaved, gpu_meshes[4]));
    scene.instances[0].mesh = &gpu_meshes[4];
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] == 255);
    scene.instances[0].world = make_transform_matrix({.p = {.y = .7f}, .q = b3Quat_identity}, {.x = .3f, .y = .3f, .z = 1});
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height * 15 / 100, pixel) && pixel[2] >= 203);
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height * 85 / 100, pixel) && pixel[2] <= 6);
    scene.instances[0].world = make_transform_matrix({.p = {.z = -2}, .q = b3Quat_identity});
    scene.camera = make_perspective(B3_PI / 2, float(compositor.width) / float(compositor.height), .1f, 10);
    CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] >= 203);
    scene.instances[0].world = {};
    scene.camera = {};
    CHECK(test_reload(compositor, scene));
    CHECK(test_render_assets(compositor, servers));
    SDL_GPUTexture* depth = compositor.depth;
    SDL_GPUGraphicsPipeline* pipeline = compositor.gbuffer.pipeline;
    for (uint32_t i = 0; i < 5; ++i)
        CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    rpmalloc_thread_statistics_t before, after;
    rpmalloc_thread_statistics(&before);
    for (uint32_t i = 0; i < 60; ++i)
        CHECK(render_frame(compositor, {scene.instances, scene.count}, scene.camera));
    rpmalloc_thread_statistics(&after);
    size_t allocations = 0, mappings = 0;
    for (uint32_t i = 0; i < 128; ++i)
        allocations += after.size_use[i].alloc_total - before.size_use[i].alloc_total;
    for (uint32_t i = 0; i < 5; ++i)
        mappings += after.span_use[i].map_calls - before.span_use[i].map_calls;
    SDL_Log("60 warm frames: %zu main-thread allocator calls, %zu new mappings (includes SDL/backend)", allocations, mappings);
    CHECK(compositor.depth == depth && compositor.gbuffer.pipeline == pipeline);
    CHECK(SDL_WaitForGPUIdle(compositor.device));
    for (GpuMesh& gpu_mesh : gpu_meshes)
        destroy_gpu_mesh(compositor.device, gpu_mesh);
    destroy_gpu_transfer_queue(transfers, compositor.device);
    destroy_rendering_compositor(compositor);
    CHECK(!compositor.device && !compositor.depth);
    destroy_arena(storage);
    return true;
}

int main()
{
    if (!initialize_memory() || !SDL_Init(SDL_INIT_VIDEO))
        return 1;
    ThreadContext context;
    if (!set_thread_context(&context))
        return 1;
    SDL_Window* window = SDL_CreateWindow("ORbit compositor test", 128, 128, SDL_WINDOW_RESIZABLE);
    if (!window)
        return 1;
    Servers servers;
    if (!create_servers(servers, {.thread_count = 2, .command_buffers = 2, .commands_per_buffer = 1}))
        return 1;
    bool success = test_rendering(window, servers);
    destroy_servers(servers);
    SDL_DestroyWindow(window);
    destroy_thread_context(context);
    SDL_Quit();
    return success ? 0 : 1;
}
