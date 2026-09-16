#ifndef ORBIT_GPU_MESH_H
#define ORBIT_GPU_MESH_H

#include <mesh.h>
#include <SDL3/SDL_gpu.h>

struct GpuMeshPrimitive
{
    uint32_t first_index = 0;
    uint32_t index_count = 0;
    int32_t vertex_offset = 0;
};

struct GpuMesh
{
    SDL_GPUBuffer* vertices = nullptr;
    SDL_GPUBuffer* indices = nullptr;
    Arena storage = {};
    Span<GpuMeshPrimitive> primitives = {};
    SDL_GPUIndexElementSize index_type = SDL_GPU_INDEXELEMENTSIZE_16BIT;
};

// Upload submits before subsequent draws. GPU resources are released through SDL's deferred release semantics.
bool upload_gpu_mesh(SDL_GPUDevice* device, const MeshAsset& source, GpuMesh& mesh);
void destroy_gpu_mesh(SDL_GPUDevice* device, GpuMesh& mesh);

#endif
