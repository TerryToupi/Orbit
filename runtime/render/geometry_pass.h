#ifndef ORBIT_GEOMETRY_PASS_H
#define ORBIT_GEOMETRY_PASS_H

#include <render/gpu_mesh.h>
#include <scene.h>

constexpr SDL_GPUTextureFormat geometry_depth_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT;
constexpr SDL_GPUVertexBufferDescription geometry_buffer = {.slot = 0, .pitch = 24, .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX};
constexpr SDL_GPUVertexAttribute geometry_attributes[] = {
    {.location = 0, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 0},
    {.location = 1, .buffer_slot = 0, .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .offset = 12}
};
constexpr SDL_GPUVertexInputState geometry_input = {
    .vertex_buffer_descriptions = &geometry_buffer, .num_vertex_buffers = 1, .vertex_attributes = geometry_attributes, .num_vertex_attributes = 2
};

struct GeometryUniforms
{
    float world[16] = {};
    float view_projection[16] = {};
};

struct GeometryDraw
{
    const GpuMesh* mesh = nullptr;
    GeometryUniforms uniforms = {};
};

SDL_GPUShader* load_geometry_shader(SDL_GPUDevice* device, const char* directory, const char* name, SDL_GPUShaderStage stage);
void draw_geometry(SDL_GPUCommandBuffer* commands, SDL_GPURenderPass* pass, Span<GeometryDraw> draws);

#endif
