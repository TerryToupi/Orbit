#include <render/rendering_compositor.h>
#include <path.h>
#include <thread_context.h>
#include <SDL3/SDL_filesystem.h>

void destroy_rendering_compositor(RenderingCompositor& compositor)
{
    if (compositor.device) {
        if (compositor.depth) SDL_ReleaseGPUTexture(compositor.device, compositor.depth);
        for (SDL_GPUTexture* color : compositor.colors)
            if (color) SDL_ReleaseGPUTexture(compositor.device, color);
        destroy_gbuffer_pass(compositor.device, compositor.gbuffer);
        destroy_depth_prepass(compositor.device, compositor.depth_pass);
        for (RenderingShader& shader : compositor.shaders)
            if (shader.shader) SDL_ReleaseGPUShader(compositor.device, shader.shader);
        if (compositor.window) SDL_ReleaseWindowFromGPUDevice(compositor.device, compositor.window);
        SDL_DestroyGPUDevice(compositor.device);
    }
    destroy_arena(compositor.shader_paths);
    compositor = {};
}

bool create_rendering_compositor(RenderingCompositor& compositor, SDL_Window* window, ArtifactCache& artifacts, const CompositorDesc& desc)
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
    SDL_GPUShaderFormat available = SDL_GetGPUShaderFormats(compositor.device);
    compositor.shader_format = (available & SDL_GPU_SHADERFORMAT_MSL) ? SDL_GPU_SHADERFORMAT_MSL :
                              ((available & SDL_GPU_SHADERFORMAT_DXIL) ? SDL_GPU_SHADERFORMAT_DXIL : SDL_GPU_SHADERFORMAT_SPIRV);
    const char* names[] = {"geometry.vert", "depth.frag", "gbuffer.frag"};
    const char* extension = compositor.shader_format == SDL_GPU_SHADERFORMAT_MSL ? "msl" :
                           (compositor.shader_format == SDL_GPU_SHADERFORMAT_DXIL ? "dxil" : "spv");
    const char* directory = path_absolute(*scratch.arena, shaders);
    if (!directory) {
        scratch_end(scratch);
        destroy_rendering_compositor(compositor);
        return false;
    }
    for (uint32_t i = 0; i < 3; ++i) {
        char filename[64];
        SDL_snprintf(filename, sizeof(filename), "%s.%s", names[i], extension);
        compositor.shaders[i].path = path_join(compositor.shader_paths, directory, filename);
    }
    scratch_end(scratch);
    compositor.artifacts = &artifacts;
    reload_rendering_shaders(compositor);
    rendering_shaders_tick(compositor);
    return true;
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

bool RenderingCompositor::render(GPUTransferQueue& transfers, Span<GeometryDraw> draws, const Mat4& view_projection)
{
    assert(device && window);
    transfers.submitted = false;
    SDL_GPUCommandBuffer* transfer_commands = nullptr;
    if (transfers.count) {
        transfer_commands = SDL_AcquireGPUCommandBuffer(device);
        if (!transfer_commands)
            return false;
    }
    SDL_GPUCommandBuffer* render_commands = SDL_AcquireGPUCommandBuffer(device);
    if (!render_commands) {
        if (transfer_commands) SDL_CancelGPUCommandBuffer(transfer_commands);
        return false;
    }
    encode_gpu_transfers(transfers, device, transfer_commands);
    SDL_GPUTexture* swapchain = nullptr;
    uint32_t frame_width = 0, frame_height = 0;
    if (depth_pass.pipeline && !SDL_WaitAndAcquireGPUSwapchainTexture(render_commands, window, &swapchain, &frame_width, &frame_height)) {
        if (transfer_commands) SDL_CancelGPUCommandBuffer(transfer_commands);
        SDL_CancelGPUCommandBuffer(render_commands);
        return false;
    }
    bool targets_ready = true;
    if (swapchain) {
        if (width != frame_width || height != frame_height)
            targets_ready = resize_targets(*this, frame_width, frame_height);
        if (targets_ready) {
            SDL_PushGPUVertexUniformData(render_commands, 0, &view_projection, sizeof(view_projection));
            depth_prepass(render_commands, depth_pass, depth, draws);
            gbuffer_pass(render_commands, gbuffer, depth, colors, draws);
            SDL_GPUBlitInfo output = {.source = {.texture = colors[0], .w = width, .h = height},
                .destination = {.texture = swapchain, .w = width, .h = height},
                .load_op = SDL_GPU_LOADOP_DONT_CARE, .filter = SDL_GPU_FILTER_NEAREST};
            SDL_BlitGPUTexture(render_commands, &output);
        }
    }
    if (transfer_commands)
        transfers.submitted = SDL_SubmitGPUCommandBuffer(transfer_commands);
    bool rendered = SDL_SubmitGPUCommandBuffer(render_commands);
    return (!transfer_commands || transfers.submitted) && rendered && targets_ready;
}
