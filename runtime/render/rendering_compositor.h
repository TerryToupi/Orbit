#ifndef ORBIT_RENDERING_COMPOSITOR_H
#define ORBIT_RENDERING_COMPOSITOR_H

#include <render/depth_prepass.h>
#include <render/gbuffer_pass.h>
#include <artifact_cache.h>
#include <render/gpu_transfer_queue.h>

struct RenderingShader
{
    const char* path = nullptr;
    SDL_GPUShader* shader = nullptr;
    ContentHash content = {};
    ContentHash observed = {}; // last attempted content, including failed GPU creation
    ContentHash pending = {};
    uint64_t request = 0;
};

struct RenderingCompositor
{
    SDL_GPUDevice* device = nullptr;
    SDL_Window* window = nullptr;
    DepthPrepass depth_pass = {};
    GBufferPass gbuffer = {};
    SDL_GPUTexture* depth = nullptr;
    SDL_GPUTexture* colors[2] = {};
    uint32_t width = 0;
    uint32_t height = 0;
    ArtifactCache* artifacts = nullptr;
    Arena shader_paths = {};
    RenderingShader shaders[3] = {}; // shared geometry vertex, depth fragment, GBuffer fragment
    SDL_GPUShaderFormat shader_format = SDL_GPU_SHADERFORMAT_INVALID;
    uint32_t shader_queued = 0;
    uint32_t shader_pending = 0;
    bool shader_load_failed = false;
    bool shader_failed = false;

    // Borrow resolved meshes/matrices. Encode the prepared upload batch, then render; submit transfer first.
    bool render(GPUTransferQueue& transfers, Span<GeometryDraw> draws, const Mat4& view_projection);
};

struct CompositorDesc
{
    const char* shader_directory = nullptr; // null selects shaders/ beside the executable
    bool debug = true;
};

// Main/window thread with a current ThreadContext. Borrows window and artifact cache; owns device/window claim.
// No copying. At shutdown wait for GPU idle, release renderer assets/transfers, then destroy before window/cache/SDL.
// Shaders load asynchronously through the application's event dispatch.
bool create_rendering_compositor(RenderingCompositor& compositor, SDL_Window* window, ArtifactCache& artifacts, const CompositorDesc& desc = {});
void destroy_rendering_compositor(RenderingCompositor& compositor);

// Call outside render, before acquiring a frame command buffer. A failed batch preserves the live shaders/pipelines.
// A refresh while a batch is pending is ignored; request again after that batch completes.
void reload_rendering_shaders(RenderingCompositor& compositor);
void rendering_shaders_tick(RenderingCompositor& compositor);
bool rendering_process_event(RenderingCompositor& compositor, const ArtifactEvent& event);

#endif
