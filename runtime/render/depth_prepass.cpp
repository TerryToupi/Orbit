#include <render/depth_prepass.h>

bool create_depth_prepass(SDL_GPUDevice* device, SDL_GPUShader* vertex, SDL_GPUShader* fragment, DepthPrepass& pass)
{
    SDL_GPUGraphicsPipelineCreateInfo info = {
        .vertex_shader = vertex, .fragment_shader = fragment, .vertex_input_state = geometry_input,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {.fill_mode = SDL_GPU_FILLMODE_FILL, .cull_mode = SDL_GPU_CULLMODE_NONE,
                            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE, .enable_depth_clip = true},
        .depth_stencil_state = {.compare_op = SDL_GPU_COMPAREOP_LESS, .enable_depth_test = true, .enable_depth_write = true},
        .target_info = {.depth_stencil_format = geometry_depth_format, .has_depth_stencil_target = true}
    };
    pass.pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    return pass.pipeline != nullptr;
}

void destroy_depth_prepass(SDL_GPUDevice* device, DepthPrepass& pass)
{
    if (pass.pipeline) SDL_ReleaseGPUGraphicsPipeline(device, pass.pipeline);
    pass = {};
}

void depth_prepass(SDL_GPUCommandBuffer* commands, const DepthPrepass& pass, SDL_GPUTexture* depth, Span<GeometryDraw> draws)
{
    SDL_GPUDepthStencilTargetInfo target = {.texture = depth, .clear_depth = 1, .load_op = SDL_GPU_LOADOP_CLEAR,
        .store_op = SDL_GPU_STOREOP_STORE, .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE, .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE, .cycle = true};
    SDL_GPURenderPass* render = SDL_BeginGPURenderPass(commands, nullptr, 0, &target);
    SDL_BindGPUGraphicsPipeline(render, pass.pipeline);
    draw_geometry(commands, render, draws);
    SDL_EndGPURenderPass(render);
}
