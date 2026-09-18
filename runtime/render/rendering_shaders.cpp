#include <render/rendering_compositor.h>
#include <SDL3/SDL_log.h>

void reload_rendering_shaders(RenderingCompositor& compositor)
{
    if (compositor.shader_queued || compositor.shader_pending)
        return;
    compositor.shader_queued = 7;
    compositor.shader_load_failed = false;
}

void rendering_shaders_tick(RenderingCompositor& compositor)
{
    if (!compositor.shader_queued || !compositor.artifacts->commands.signal->accepting)
        return;
    ArtifactCommandBuffer* commands = nullptr;
    for (uint32_t i = 0; i < 3; ++i) {
        if (!(compositor.shader_queued & (1u << i)))
            continue;
        if (commands && commands->count == commands->owner->command_limit) {
            commands_submit(*commands);
            commands = nullptr;
        }
        if (!commands) {
            commands = commands_begin(compositor.artifacts->commands);
            if (!commands)
                break;
        }
        compositor.shaders[i].request = file_load(*commands, compositor.shaders[i].path);
        compositor.shader_queued &= ~(1u << i);
        compositor.shader_pending |= 1u << i;
    }
    if (commands)
        commands_submit(*commands);
}

static bool replace_pipelines(RenderingCompositor& compositor)
{
    SDL_GPUShader* replacement[3] = {};
    SDL_GPUShader* shaders[3] = {};
    DepthPrepass depth = {};
    GBufferPass gbuffer = {};
    bool success = true;
    for (uint32_t i = 0; i < 3; ++i) {
        RenderingShader& source = compositor.shaders[i];
        shaders[i] = source.shader;
        if (source.shader && source.pending == source.content)
            continue;
        ContentView bytes = content_get(*compositor.artifacts->content, source.pending);
        if (!bytes.size) {
            SDL_SetError("Empty rendering shader: %s", source.path);
            success = false;
            break;
        }
        SDL_GPUShaderCreateInfo info = {
            .code_size = bytes.size, .code = bytes.data,
            .entrypoint = compositor.shader_format == SDL_GPU_SHADERFORMAT_MSL ? "main0" : "main",
            .format = compositor.shader_format, .stage = i == 0 ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT,
            .num_uniform_buffers = i == 0 ? 2u : 0u
        };
        replacement[i] = SDL_CreateGPUShader(compositor.device, &info);
        if (!replacement[i]) {
            success = false;
            break;
        }
        shaders[i] = replacement[i];
    }
    if (success && (replacement[0] || replacement[1]))
        success = create_depth_prepass(compositor.device, shaders[0], shaders[1], depth);
    if (success && (replacement[0] || replacement[2]))
        success = create_gbuffer_pass(compositor.device, shaders[0], shaders[2], gbuffer);
    if (success) {
        if (depth.pipeline) {
            destroy_depth_prepass(compositor.device, compositor.depth_pass);
            compositor.depth_pass = depth;
        }
        if (gbuffer.pipeline) {
            destroy_gbuffer_pass(compositor.device, compositor.gbuffer);
            compositor.gbuffer = gbuffer;
        }
        for (uint32_t i = 0; i < 3; ++i) {
            if (!replacement[i])
                continue;
            if (compositor.shaders[i].shader)
                SDL_ReleaseGPUShader(compositor.device, compositor.shaders[i].shader);
            compositor.shaders[i].shader = replacement[i];
            compositor.shaders[i].content = compositor.shaders[i].pending;
        }
    } else {
        SDL_Log("Rendering shader replacement failed: %s", SDL_GetError());
        destroy_depth_prepass(compositor.device, depth);
        destroy_gbuffer_pass(compositor.device, gbuffer);
        for (SDL_GPUShader* shader : replacement)
            if (shader) SDL_ReleaseGPUShader(compositor.device, shader);
    }
    return success;
}

bool rendering_process_event(RenderingCompositor& compositor, const ArtifactEvent& event)
{
    if (event.kind != ArtifactKind::File)
        return false;
    for (uint32_t i = 0; i < 3; ++i) {
        RenderingShader& source = compositor.shaders[i];
        if (!(compositor.shader_pending & (1u << i)) || source.request != event.request)
            continue;
        compositor.shader_pending &= ~(1u << i);
        if (event.result.state != ArtifactState::Ready) {
            compositor.shader_load_failed = true;
            compositor.shader_failed = true;
            SDL_Log("Rendering shader load failed: %s (%s)", source.path, event.error);
        } else {
            source.pending = event.result.content;
        }
        if (!compositor.shader_pending && !compositor.shader_queued && !compositor.shader_load_failed) {
            bool changed = false;
            bool stale = false;
            for (RenderingShader& shader : compositor.shaders) {
                changed |= shader.pending != shader.observed;
                stale |= shader.pending != shader.content || !shader.shader;
                shader.observed = shader.pending;
            }
            compositor.shader_failed = changed ? !replace_pipelines(compositor) : stale;
        }
        return true;
    }
    return false;
}
