#ifndef ORBIT_DEPTH_PREPASS_H
#define ORBIT_DEPTH_PREPASS_H

#include <render/geometry_pass.h>

struct DepthPrepass
{
    SDL_GPUShader* vertex = nullptr;
    SDL_GPUShader* fragment = nullptr;
    SDL_GPUGraphicsPipeline* pipeline = nullptr;
};

bool create_depth_prepass(SDL_GPUDevice* device, const char* shaders, DepthPrepass& pass);
void destroy_depth_prepass(SDL_GPUDevice* device, DepthPrepass& pass);
void depth_prepass(SDL_GPUCommandBuffer* commands, const DepthPrepass& pass, SDL_GPUTexture* depth, Span<GeometryDraw> draws);

#endif
