#include <render/geometry_pass.h>
#include <path.h>
#include <thread_context.h>
#include <SDL3/SDL_iostream.h>

SDL_GPUShader* load_geometry_shader(SDL_GPUDevice* device, const char* directory, const char* name, SDL_GPUShaderStage stage)
{
    ArenaTemp scratch = scratch_begin();
    SDL_GPUShaderFormat available = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = (available & SDL_GPU_SHADERFORMAT_MSL) ? SDL_GPU_SHADERFORMAT_MSL :
                                ((available & SDL_GPU_SHADERFORMAT_DXIL) ? SDL_GPU_SHADERFORMAT_DXIL : SDL_GPU_SHADERFORMAT_SPIRV);
    char filename[128];
    SDL_snprintf(filename, sizeof(filename), "%s.%s", name, format == SDL_GPU_SHADERFORMAT_MSL ? "msl" :
                 (format == SDL_GPU_SHADERFORMAT_DXIL ? "dxil" : "spv"));
    size_t size;
    void* bytes = SDL_LoadFile(path_join(*scratch.arena, directory, filename), &size);
    scratch_end(scratch);
    if (!bytes)
        return nullptr;
    SDL_GPUShaderCreateInfo info = {.code_size = size, .code = static_cast<const uint8_t*>(bytes),
        .entrypoint = format == SDL_GPU_SHADERFORMAT_MSL ? "main0" : "main", .format = format, .stage = stage,
        .num_uniform_buffers = stage == SDL_GPU_SHADERSTAGE_VERTEX ? 1u : 0u};
    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
    SDL_free(bytes);
    return shader;
}

void draw_geometry(SDL_GPUCommandBuffer* commands, SDL_GPURenderPass* pass, Span<GeometryDraw> draws)
{
    for (size_t i = 0; i < draws.size; ++i) {
        const GeometryDraw& draw = draws.data[i];
        SDL_PushGPUVertexUniformData(commands, 0, &draw.uniforms, sizeof(draw.uniforms));
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
