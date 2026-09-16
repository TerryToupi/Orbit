#include <render/rendering_compositor.h>
#include <path.h>
#include <thread_context.h>
#include <SDL3/SDL_filesystem.h>

void destroy_rendering_compositor(RenderingCompositor& compositor)
{
    if (compositor.device) {
        SDL_WaitForGPUIdle(compositor.device);
        for (uint32_t i = 0; i < compositor.mesh_count; ++i)
            destroy_gpu_mesh(compositor.device, compositor.meshes[i].mesh);
        if (compositor.depth) SDL_ReleaseGPUTexture(compositor.device, compositor.depth);
        for (SDL_GPUTexture* color : compositor.colors)
            if (color) SDL_ReleaseGPUTexture(compositor.device, color);
        destroy_gbuffer_pass(compositor.device, compositor.gbuffer);
        destroy_depth_prepass(compositor.device, compositor.depth_pass);
        if (compositor.window) SDL_ReleaseWindowFromGPUDevice(compositor.device, compositor.window);
        SDL_DestroyGPUDevice(compositor.device);
    }
    SDL_free(compositor.meshes);
    SDL_free(compositor.draws);
    compositor = {};
}

bool create_rendering_compositor(RenderingCompositor& compositor, SDL_Window* window, const CompositorDesc& desc)
{
    assert(window && !compositor.device);
    SDL_GPUShaderFormat formats = SDL_GPU_SHADERFORMAT_SPIRV;
#if defined(__APPLE__)
    formats |= SDL_GPU_SHADERFORMAT_MSL;
#elif defined(_WIN32)
    formats |= SDL_GPU_SHADERFORMAT_DXIL;
#endif
    compositor.device = SDL_CreateGPUDevice(formats, desc.debug, nullptr);
    if (!compositor.device)
        return false;
    if (!SDL_ClaimWindowForGPUDevice(compositor.device, window)) {
        destroy_rendering_compositor(compositor);
        return false;
    }
    compositor.window = window;
    if (!SDL_SetGPUSwapchainParameters(compositor.device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC)) {
        destroy_rendering_compositor(compositor);
        return false;
    }
    ArenaTemp scratch = scratch_begin();
    const char* shaders = desc.shader_directory;
    if (!shaders) {
        const char* base = SDL_GetBasePath();
        if (!base) {
            scratch_end(scratch);
            destroy_rendering_compositor(compositor);
            return false;
        }
        shaders = path_join(*scratch.arena, base, "shaders");
    }
    bool created = create_depth_prepass(compositor.device, shaders, compositor.depth_pass) &&
                   create_gbuffer_pass(compositor.device, shaders, compositor.gbuffer);
    scratch_end(scratch);
    if (!created)
        destroy_rendering_compositor(compositor);
    return created;
}

static bool resize_targets(RenderingCompositor& compositor, uint32_t width, uint32_t height)
{
    SDL_GPUTextureCreateInfo info = {.type = SDL_GPU_TEXTURETYPE_2D, .format = geometry_depth_format,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET, .width = width, .height = height, .layer_count_or_depth = 1, .num_levels = 1};
    SDL_GPUTexture* depth = SDL_CreateGPUTexture(compositor.device, &info);
    if (!depth)
        return false;
    SDL_GPUTexture* colors[2] = {};
    for (uint32_t i = 0; i < 2; ++i) {
        info.format = gbuffer_formats[i];
        info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
        colors[i] = SDL_CreateGPUTexture(compositor.device, &info);
        if (!colors[i]) {
            SDL_ReleaseGPUTexture(compositor.device, depth);
            if (colors[0]) SDL_ReleaseGPUTexture(compositor.device, colors[0]);
            return false;
        }
    }
    if (compositor.depth) SDL_ReleaseGPUTexture(compositor.device, compositor.depth);
    compositor.depth = depth;
    for (uint32_t i = 0; i < 2; ++i) {
        if (compositor.colors[i]) SDL_ReleaseGPUTexture(compositor.device, compositor.colors[i]);
        compositor.colors[i] = colors[i];
    }
    compositor.width = width;
    compositor.height = height;
    return true;
}

bool RenderingCompositor::render(const Scene& scene)
{
    assert(device && window);
    Span<SceneMesh> instances = scene.meshes();
    if (instances.size > draw_capacity) {
        draw_capacity = uint32_t((instances.size + 127) / 128 * 128);
        draws = static_cast<GeometryDraw*>(SDL_realloc(draws, draw_capacity * sizeof(GeometryDraw)));
    }
    // Reserve before capturing pointers into the cache for this frame.
    if (mesh_count + instances.size > mesh_capacity) {
        mesh_capacity = uint32_t((mesh_count + instances.size + 127) / 128 * 128);
        meshes = static_cast<CachedGpuMesh*>(SDL_realloc(meshes, mesh_capacity * sizeof(CachedGpuMesh)));
    }
    uint32_t draw_count = 0;
    const SceneMatrix& camera = scene.view_projection();
    for (size_t i = 0; i < instances.size; ++i) {
        const SceneMesh& instance = instances.data[i];
        if (!instance.mesh)
            continue;
        uint32_t index = 0;
        while (index < mesh_count && meshes[index].source != instance.mesh)
            ++index;
        if (index == mesh_count) {
            meshes[index] = {.source = instance.mesh};
            if (!upload_gpu_mesh(device, *instance.mesh, meshes[index].mesh))
                return false;
            ++mesh_count;
        }
        GeometryDraw& draw = draws[draw_count++];
        draw.mesh = &meshes[index].mesh;
        SDL_memcpy(draw.uniforms.world, instance.world, sizeof(instance.world));
        SDL_memcpy(draw.uniforms.view_projection, camera, sizeof(camera));
    }
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(device);
    if (!commands)
        return false;
    SDL_GPUTexture* swapchain = nullptr;
    uint32_t frame_width, frame_height;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(commands, window, &swapchain, &frame_width, &frame_height)) {
        SDL_CancelGPUCommandBuffer(commands);
        return false;
    }
    if (!swapchain)
        return SDL_CancelGPUCommandBuffer(commands);
    if ((width != frame_width || height != frame_height) && !resize_targets(*this, frame_width, frame_height)) {
        SDL_SubmitGPUCommandBuffer(commands);
        return false;
    }
    depth_prepass(commands, depth_pass, depth, {draws, draw_count});
    gbuffer_pass(commands, gbuffer, depth, colors, {draws, draw_count});
    SDL_GPUBlitInfo output = {.source = {.texture = colors[0], .w = width, .h = height},
        .destination = {.texture = swapchain, .w = width, .h = height}, .load_op = SDL_GPU_LOADOP_DONT_CARE, .filter = SDL_GPU_FILTER_NEAREST};
    SDL_BlitGPUTexture(commands, &output);
    return SDL_SubmitGPUCommandBuffer(commands);
}
