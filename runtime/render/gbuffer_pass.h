#ifndef ORBIT_GBUFFER_PASS_H
#define ORBIT_GBUFFER_PASS_H

#include <render/geometry_pass.h>

// Linear debug base color; encoded world normal (xyz * .5 + .5) and roughness.
constexpr SDL_GPUTextureFormat gbuffer_formats[] = {SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM};

struct GBufferPass
{
    SDL_GPUGraphicsPipeline* pipeline = nullptr;
};

bool create_gbuffer_pass(SDL_GPUDevice* device, SDL_GPUShader* vertex, SDL_GPUShader* fragment, GBufferPass& pass);
void destroy_gbuffer_pass(SDL_GPUDevice* device, GBufferPass& pass);
void gbuffer_pass(SDL_GPUCommandBuffer* commands, const GBufferPass& pass, SDL_GPUTexture* depth,
                  SDL_GPUTexture* const (&colors)[2], Span<GeometryDraw> draws);

#endif
