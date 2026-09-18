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
    size_t vertex_bytes = 0;
    Span<GpuMeshPrimitive> primitives = {}; // owned allocation
    SDL_GPUIndexElementSize index_type = SDL_GPU_INDEXELEMENTSIZE_16BIT;
};

// Creation requires an empty destination. Pack the same source into spans sized for vertex_bytes and source.indices.size.
// Packing writes the current geometry ABI during upload preparation.
// Release after recording/submitting all uses, before device destruction. SDL defers GPU release until queued uses finish.
bool create_gpu_mesh(SDL_GPUDevice* device, const MeshAsset& source, GpuMesh& mesh);
void destroy_gpu_mesh(SDL_GPUDevice* device, GpuMesh& mesh);
bool pack_gpu_mesh(const MeshAsset& source, GpuMesh& mesh, uint8_t* vertices, uint8_t* indices);

#endif
