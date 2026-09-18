#include <render/gbuffer_pass.h>

bool create_gbuffer_pass(SDL_GPUDevice* device, SDL_GPUShader* vertex, SDL_GPUShader* fragment, GBufferPass& pass)
{
    SDL_GPUColorTargetDescription targets[] = {{.format = gbuffer_formats[0]}, {.format = gbuffer_formats[1]}};
    SDL_GPUGraphicsPipelineCreateInfo info = {
        .vertex_shader = vertex, .fragment_shader = fragment, .vertex_input_state = geometry_input,
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {.fill_mode = SDL_GPU_FILLMODE_FILL, .cull_mode = SDL_GPU_CULLMODE_NONE,
                            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE, .enable_depth_clip = true},
        .depth_stencil_state = {.compare_op = SDL_GPU_COMPAREOP_EQUAL, .enable_depth_test = true},
        .target_info = {.color_target_descriptions = targets, .num_color_targets = 2,
                        .depth_stencil_format = geometry_depth_format, .has_depth_stencil_target = true}
    };
    pass.pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    return pass.pipeline != nullptr;
}

void destroy_gbuffer_pass(SDL_GPUDevice* device, GBufferPass& pass)
{
    if (pass.pipeline) SDL_ReleaseGPUGraphicsPipeline(device, pass.pipeline);
    pass = {};
}

void gbuffer_pass(SDL_GPUCommandBuffer* commands, const GBufferPass& pass, SDL_GPUTexture* depth,
                  SDL_GPUTexture* const (&colors)[2], Span<GeometryDraw> draws)
{
    SDL_GPUColorTargetInfo targets[] = {
        {.texture = colors[0], .clear_color = {.r = .02f, .g = .02f, .b = .02f, .a = 1},
         .load_op = SDL_GPU_LOADOP_CLEAR, .store_op = SDL_GPU_STOREOP_STORE, .cycle = true},
        {.texture = colors[1], .load_op = SDL_GPU_LOADOP_CLEAR, .store_op = SDL_GPU_STOREOP_STORE, .cycle = true}
    };
    SDL_GPUDepthStencilTargetInfo depth_target = {.texture = depth, .load_op = SDL_GPU_LOADOP_LOAD,
        .store_op = SDL_GPU_STOREOP_STORE, .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE, .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE};
    SDL_GPURenderPass* render = SDL_BeginGPURenderPass(commands, targets, 2, &depth_target);
    SDL_BindGPUGraphicsPipeline(render, pass.pipeline);
    draw_geometry(commands, render, draws);
    SDL_EndGPURenderPass(render);
}
