#include <render/gpu_mesh.h>
#include <render/geometry_pass.h>
#include <SDL3/SDL_stdinc.h>

void destroy_gpu_mesh(SDL_GPUDevice* device, GpuMesh& mesh)
{
    if (mesh.vertices) SDL_ReleaseGPUBuffer(device, mesh.vertices);
    if (mesh.indices) SDL_ReleaseGPUBuffer(device, mesh.indices);
    destroy_arena(mesh.storage);
    mesh = {};
}

bool upload_gpu_mesh(SDL_GPUDevice* device, const MeshAsset& source, GpuMesh& mesh)
{
    size_t vertex_count = 0;
    for (size_t p = 0; p < source.primitives.size; ++p)
        vertex_count += source.primitives.data[p].vertex_count;
    if (!vertex_count || !source.indices.size)
        return SDL_SetError("Cannot upload empty mesh geometry");
    SDL_GPUBufferCreateInfo vertex_info = {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = uint32_t(vertex_count * geometry_buffer.pitch)};
    SDL_GPUBufferCreateInfo index_info = {.usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = uint32_t(source.indices.size)};
    mesh.vertices = SDL_CreateGPUBuffer(device, &vertex_info);
    if (!mesh.vertices)
        return false;
    mesh.indices = SDL_CreateGPUBuffer(device, &index_info);
    if (!mesh.indices) {
        destroy_gpu_mesh(device, mesh);
        return false;
    }
    SDL_GPUTransferBufferCreateInfo transfer_info = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = vertex_info.size + index_info.size};
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(device, &transfer_info);
    if (!transfer) {
        destroy_gpu_mesh(device, mesh);
        return false;
    }
    uint8_t* mapped = static_cast<uint8_t*>(SDL_MapGPUTransferBuffer(device, transfer, false));
    if (!mapped) {
        SDL_ReleaseGPUTransferBuffer(device, transfer);
        destroy_gpu_mesh(device, mesh);
        return false;
    }
    GpuMeshPrimitive* primitives = arena_allocate<GpuMeshPrimitive>(mesh.storage, source.primitives.size);
    mesh.primitives = {primitives, source.primitives.size};
    mesh.index_type = source.index_type == MeshIndexType::UInt16 ? SDL_GPU_INDEXELEMENTSIZE_16BIT : SDL_GPU_INDEXELEMENTSIZE_32BIT;
    uint32_t vertex_base = 0;
    bool valid = true;
    for (size_t p = 0; p < source.primitives.size && valid; ++p) {
        const MeshPrimitive& primitive = source.primitives.data[p];
        assert(primitive.first_stream + primitive.stream_count <= source.streams.size);
        const VertexStream* streams[2] = {};
        const VertexAttribute* attributes[2] = {};
        for (uint32_t s = 0; s < primitive.stream_count; ++s) {
            const VertexStream& stream = source.streams.data[primitive.first_stream + s];
            for (size_t a = 0; a < stream.attributes.size; ++a) {
                const VertexAttribute& attribute = stream.attributes.data[a];
                if (attribute.semantic != VertexSemantic::Position && attribute.semantic != VertexSemantic::Normal)
                    continue;
                uint32_t slot = attribute.semantic == VertexSemantic::Position ? 0 : 1;
                if (attributes[slot] || attribute.format != VertexFormat::Float3 || stream.stride < 12 ||
                    attribute.offset > stream.stride - 12 || !primitive.vertex_count ||
                    stream.data.size < size_t(primitive.vertex_count - 1) * stream.stride + attribute.offset + 12) {
                    valid = false;
                    break;
                }
                streams[slot] = &stream;
                attributes[slot] = &attribute;
            }
        }
        if (!attributes[0] || !valid) {
            valid = false;
            break;
        }
        for (uint32_t v = 0; v < primitive.vertex_count; ++v) {
            uint8_t* vertex = mapped + size_t(vertex_base + v) * geometry_buffer.pitch;
            SDL_memcpy(vertex, streams[0]->data.data + size_t(v) * streams[0]->stride + attributes[0]->offset, 12);
            if (attributes[1])
                SDL_memcpy(vertex + 12, streams[1]->data.data + size_t(v) * streams[1]->stride + attributes[1]->offset, 12);
            else
                SDL_memset(vertex + 12, 0, 12);
        }
        primitives[p] = {.first_index = primitive.first_index, .index_count = primitive.index_count, .vertex_offset = int32_t(vertex_base)};
        vertex_base += primitive.vertex_count;
    }
    SDL_memcpy(mapped + vertex_info.size, source.indices.data, source.indices.size);
    SDL_UnmapGPUTransferBuffer(device, transfer);
    if (!valid) {
        SDL_ReleaseGPUTransferBuffer(device, transfer);
        destroy_gpu_mesh(device, mesh);
        return SDL_SetError("Mesh requires valid Float3 positions and optional Float3 normals");
    }
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(device);
    if (!commands) {
        SDL_ReleaseGPUTransferBuffer(device, transfer);
        destroy_gpu_mesh(device, mesh);
        return false;
    }
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUTransferBufferLocation from = {.transfer_buffer = transfer};
    SDL_GPUBufferRegion to = {.buffer = mesh.vertices, .size = vertex_info.size};
    SDL_UploadToGPUBuffer(copy, &from, &to, false);
    from.offset = vertex_info.size;
    to = {.buffer = mesh.indices, .size = index_info.size};
    SDL_UploadToGPUBuffer(copy, &from, &to, false);
    SDL_EndGPUCopyPass(copy);
    bool submitted = SDL_SubmitGPUCommandBuffer(commands);
    SDL_ReleaseGPUTransferBuffer(device, transfer);
    if (!submitted)
        destroy_gpu_mesh(device, mesh);
    return submitted;
}
