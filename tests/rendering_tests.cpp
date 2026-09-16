#include <render/rendering_compositor.h>
#include <memory_setup.h>
#include <thread_context.h>
#include <SDL3/SDL.h>

#define CHECK(x) do { if (!(x)) { SDL_Log("%s:%d: %s (%s)", __FILE__, __LINE__, #x, SDL_GetError()); return false; } } while (0)

struct TestScene final : Scene
{
    SceneMesh instances[2] = {};
    size_t count = 1;
    SceneMatrix camera = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    Span<SceneMesh> meshes() const override { return {instances, count}; }
    const SceneMatrix& view_projection() const override { return camera; }
};

static bool read_pixel(RenderingCompositor& compositor, uint32_t target, uint32_t x, uint32_t y, uint8_t (&pixel)[4])
{
    uint32_t row = (compositor.width * 4 + 255) / 256 * 256;
    SDL_GPUTransferBufferCreateInfo info = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, .size = row * compositor.height};
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(compositor.device, &info);
    CHECK(transfer);
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(compositor.device);
    CHECK(commands);
    SDL_GPUCopyPass* copy = SDL_BeginGPUCopyPass(commands);
    SDL_GPUTextureRegion from = {.texture = compositor.colors[target], .w = compositor.width, .h = compositor.height, .d = 1};
    SDL_GPUTextureTransferInfo to = {.transfer_buffer = transfer, .pixels_per_row = row / 4, .rows_per_layer = compositor.height};
    SDL_DownloadFromGPUTexture(copy, &from, &to);
    SDL_EndGPUCopyPass(copy);
    SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(commands);
    CHECK(fence && SDL_WaitForGPUFences(compositor.device, true, &fence, 1));
    const uint8_t* mapped = static_cast<const uint8_t*>(SDL_MapGPUTransferBuffer(compositor.device, transfer, false));
    CHECK(mapped);
    SDL_memcpy(pixel, mapped + size_t(y) * row + x * 4, 4);
    SDL_UnmapGPUTransferBuffer(compositor.device, transfer);
    SDL_ReleaseGPUFence(compositor.device, fence);
    SDL_ReleaseGPUTransferBuffer(compositor.device, transfer);
    return true;
}

static bool render_visible(RenderingCompositor& compositor, const Scene& scene)
{
    for (uint32_t i = 0; i < 100; ++i) {
        SDL_PumpEvents();
        CHECK(compositor.render(scene));
        if (compositor.width)
            return true;
        SDL_Delay(10);
    }
    return false;
}

static bool test_rendering(SDL_Window* window)
{
    RenderingCompositor compositor;
    CHECK(!create_rendering_compositor(compositor, window, {.shader_directory = "/nonexistent/orbit-shaders"}));
    CHECK(!compositor.device && !compositor.window);
    CHECK(create_rendering_compositor(compositor, window, {.shader_directory = ORBIT_SHADER_DIRECTORY}));
    SDL_Log("Rendering tests on SDL GPU driver: %s", SDL_GetGPUDeviceDriver(compositor.device));

    Arena storage;
    const float near_positions[] = {-.7f, -.7f, .2f, .7f, -.7f, .2f, 0, .7f, .2f};
    const float far_positions[] = {-.7f, -.7f, .8f, .7f, -.7f, .8f, 0, .7f, .8f};
    const float normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1};
    const float far_normals[] = {1, 0, 0, 1, 0, 0, 1, 0, 0};
    const uint32_t indices[] = {0, 1, 2};
    ImportedPrimitive primitives[] = {
        {.positions = near_positions, .normals = normals, .indices = indices},
        {.positions = far_positions, .normals = far_normals, .indices = indices}
    };
    MeshAsset mesh;
    char error[160] = {};
    CHECK(build_mesh_asset(storage, {.primitives = primitives}, mesh, error));
    TestScene scene;
    scene.instances[0].mesh = &mesh;
    CHECK(render_visible(compositor, scene));
    CHECK(compositor.mesh_count == 1 && compositor.meshes[0].mesh.primitives.size == 2);
    uint8_t pixel[4];
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel));
    CHECK(pixel[0] >= 50 && pixel[0] <= 52 && pixel[2] >= 203 && pixel[3] == 255);
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel));
    CHECK(pixel[0] >= 127 && pixel[0] <= 128 && pixel[2] == 255 && pixel[3] >= 203);

    const GpuMesh& gpu = compositor.meshes[0].mesh;
    GpuMesh far_only = {.vertices = gpu.vertices, .indices = gpu.indices, .primitives = {gpu.primitives.data + 1, 1}, .index_type = gpu.index_type};
    GeometryDraw far_draw = {.mesh = &far_only, .uniforms = compositor.draws[0].uniforms};
    SDL_GPUCommandBuffer* commands = SDL_AcquireGPUCommandBuffer(compositor.device);
    CHECK(commands);
    gbuffer_pass(commands, compositor.gbuffer, compositor.depth, compositor.colors, {&far_draw, 1});
    CHECK(SDL_SubmitGPUCommandBuffer(commands));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel));
    CHECK(pixel[0] <= 6 && pixel[2] <= 6);

    scene.instances[0].world[12] = .9f;
    CHECK(compositor.render(scene));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] <= 6);
    scene.camera[12] = -.9f;
    CHECK(compositor.render(scene));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] >= 203);
    CHECK(compositor.mesh_count == 1);

    scene.instances[1] = scene.instances[0];
    scene.count = 2;
    CHECK(compositor.render(scene) && compositor.mesh_count == 1);
    scene.count = 0;
    CHECK(compositor.render(scene));
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] <= 6);
    scene.count = 1;
    scene.instances[0].world[12] = 0;
    scene.camera[12] = 0;

    float diagonal[] = {.70710678f, .70710678f, 0, .70710678f, .70710678f, 0, .70710678f, .70710678f, 0};
    ImportedPrimitive transformed = {.positions = near_positions, .normals = diagonal, .indices = indices};
    MeshAsset replacement;
    CHECK(build_mesh_asset(storage, {.primitives = {&transformed, 1}}, replacement, error));
    scene.instances[0].mesh = &replacement;
    scene.instances[0].world[0] = -2;
    scene.instances[0].world[5] = .5f;
    scene.camera[0] = -.5f;
    scene.camera[5] = 2;
    CHECK(compositor.render(scene) && compositor.mesh_count == 2);
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel));
    CHECK(pixel[0] >= 95 && pixel[0] <= 98 && pixel[1] >= 250 && pixel[2] >= 127 && pixel[2] <= 128);

    uint32_t old_width = compositor.width;
    CHECK(SDL_SetWindowSize(window, 192, 160));
    for (uint32_t i = 0; i < 100 && compositor.width == old_width; ++i) {
        SDL_PumpEvents();
        CHECK(compositor.render(scene));
        SDL_Delay(10);
    }
    CHECK(compositor.width != old_width);
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel) && pixel[1] >= 250);
    scene.instances[0].world[0] = 1;
    scene.instances[0].world[5] = 1;
    scene.camera[0] = 1;
    scene.camera[5] = 1;
    const uint32_t wide_indices[] = {0, 1, 2, 0, 1, 2};
    MeshAsset wide = mesh;
    wide.indices = {reinterpret_cast<const uint8_t*>(wide_indices), sizeof(wide_indices)};
    wide.index_type = MeshIndexType::UInt32;
    scene.instances[0].mesh = &wide;
    CHECK(compositor.render(scene));
    CHECK(compositor.meshes[2].mesh.index_type == SDL_GPU_INDEXELEMENTSIZE_32BIT);
    CHECK(read_pixel(compositor, 0, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] >= 203);

    ImportedPrimitive bare = {.positions = near_positions, .indices = indices};
    MeshAsset positions_only;
    CHECK(build_mesh_asset(storage, {.primitives = {&bare, 1}}, positions_only, error));
    scene.instances[0].mesh = &positions_only;
    CHECK(compositor.render(scene));
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] == 255);

    float packed[18];
    for (uint32_t i = 0; i < 3; ++i) {
        SDL_memcpy(packed + i * 6, near_positions + i * 3, 12);
        SDL_memcpy(packed + i * 6 + 3, normals + i * 3, 12);
    }
    VertexAttribute attributes[] = {
        {.semantic = VertexSemantic::Position, .format = VertexFormat::Float3},
        {.semantic = VertexSemantic::Normal, .format = VertexFormat::Float3, .offset = 12}
    };
    VertexStream stream = {.data = {reinterpret_cast<const uint8_t*>(packed), sizeof(packed)}, .attributes = attributes, .stride = 24};
    MeshAsset interleaved = positions_only;
    interleaved.streams = {&stream, 1};
    scene.instances[0].mesh = &interleaved;
    CHECK(compositor.render(scene));
    CHECK(read_pixel(compositor, 1, compositor.width / 2, compositor.height / 2, pixel) && pixel[2] == 255);
    GeometryDraw* draws = compositor.draws;
    CachedGpuMesh* cached = compositor.meshes;
    for (uint32_t i = 0; i < 5; ++i)
        CHECK(compositor.render(scene));
    CHECK(compositor.draws == draws && compositor.meshes == cached && compositor.mesh_count == 5);
    destroy_rendering_compositor(compositor);
    CHECK(!compositor.device && !compositor.depth && !compositor.meshes);
    destroy_arena(storage);
    return true;
}

int main()
{
    if (!initialize_memory() || !SDL_Init(SDL_INIT_VIDEO))
        return 1;
    ThreadContext context;
    if (!set_thread_context(&context))
        return 1;
    SDL_Window* window = SDL_CreateWindow("ORbit compositor test", 128, 128, SDL_WINDOW_RESIZABLE);
    if (!window)
        return 1;
    bool success = test_rendering(window);
    SDL_DestroyWindow(window);
    destroy_thread_context(context);
    SDL_Quit();
    return success ? 0 : 1;
}
