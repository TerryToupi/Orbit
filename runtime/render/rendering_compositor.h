#ifndef ORBIT_RENDERING_COMPOSITOR_H
#define ORBIT_RENDERING_COMPOSITOR_H

#include <render/depth_prepass.h>
#include <render/gbuffer_pass.h>

struct CachedGpuMesh
{
    const MeshAsset* source = nullptr;
    GpuMesh mesh = {};
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
    CachedGpuMesh* meshes = nullptr;
    uint32_t mesh_count = 0;
    uint32_t mesh_capacity = 0;
    GeometryDraw* draws = nullptr;
    uint32_t draw_capacity = 0;

    bool render(const Scene& scene);
};

struct CompositorDesc
{
    const char* shader_directory = nullptr; // null selects shaders/ beside the executable
    bool debug = true;
};

// Main/window thread only, with a current ThreadContext. Borrows the application window; owns its GPU claim.
// No copying. Destroy before the window, mesh backing storage, or SDL. Failed creation cleans up partial resources.
bool create_rendering_compositor(RenderingCompositor& compositor, SDL_Window* window, const CompositorDesc& desc = {});
void destroy_rendering_compositor(RenderingCompositor& compositor);

#endif
