#include <render/geometry_pass.h>

void draw_geometry(SDL_GPUCommandBuffer* commands, SDL_GPURenderPass* pass, Span<GeometryDraw> draws)
{
    for (size_t i = 0; i < draws.size; ++i) {
        const GeometryDraw& draw = draws.data[i];
        if (!draw.mesh)
            continue;
        SDL_PushGPUVertexUniformData(commands, 1, &draw.world, sizeof(draw.world));
        SDL_GPUBufferBinding vertices = {.buffer = draw.mesh->vertices};
        SDL_GPUBufferBinding indices = {.buffer = draw.mesh->indices};
        SDL_BindGPUVertexBuffers(pass, 0, &vertices, 1);
        SDL_BindGPUIndexBuffer(pass, &indices, draw.mesh->index_type);
        for (size_t p = 0; p < draw.mesh->primitives.size; ++p) {
            const GpuMeshPrimitive& primitive = draw.mesh->primitives.data[p];
            SDL_DrawGPUIndexedPrimitives(pass, primitive.index_count, 1, primitive.first_index, primitive.vertex_offset, 0);
        }
    }
}
