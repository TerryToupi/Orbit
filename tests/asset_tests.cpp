#include <asset_factory.h>
#include <render/render_asset_cache.h>
#include <gltf_import.h>
#include <path.h>
#include <servers.h>
#include <thread_context.h>
#include <memory_setup.h>
#include <SDL3/SDL.h>
#include <type_traits>

#define CHECK(x) do { if (!(x)) { SDL_Log("%s:%d: %s (%s)", __FILE__, __LINE__, #x, SDL_GetError()); return false; } } while (0)

static_assert(sizeof(AssetID) == 8 && sizeof(AssetHandle<MeshAsset>) == 8);
static_assert(std::is_trivially_copyable_v<MeshAsset> && std::is_trivially_copyable_v<ImportedScene>);

#include "mesh_fixture.h"

static bool write_bytes(const char* path, Span<uint8_t> bytes)
{
    SDL_IOStream* file = SDL_IOFromFile(path, "wb");
    CHECK(file);
    size_t written = SDL_WriteIO(file, bytes.data, bytes.size);
    bool closed = SDL_CloseIO(file);
    CHECK(written == bytes.size && closed);
    return true;
}

static bool test_paths()
{
    Arena arena;
    CHECK(SDL_strcmp(path_normalize(arena, "models/.././triangle.glb"), "triangle.glb") == 0);
    CHECK(SDL_strcmp(path_normalize(arena, "models\\part//../triangle.glb"), "models/triangle.glb") == 0);
    CHECK(SDL_strcmp(path_normalize(arena, "/a/b/../../c/"), "/c") == 0);
    CHECK(SDL_strcmp(path_normalize(arena, "c:\\a\\..\\b"), "C:/b") == 0);
    CHECK(!path_normalize(arena, "../outside") && !path_normalize(arena, "a/../../outside"));
    CHECK(!path_normalize(arena, "") && !path_normalize(arena, "C:relative"));
    CHECK(asset_id("triangle.glb") == 8606554518927658136ull);
    CHECK(asset_id("triangle.glb") != asset_id("Triangle.glb"));
    destroy_arena(arena);
    return true;
}

static const float* attribute(const MeshAsset& mesh, uint32_t primitive, VertexSemantic semantic)
{
    const MeshPrimitive& p = mesh.primitives.data[primitive];
    for (uint32_t i = 0; i < p.stream_count; ++i) {
        const VertexStream& stream = mesh.streams.data[p.first_stream + i];
        for (size_t a = 0; a < stream.attributes.size; ++a) {
            if (stream.attributes.data[a].semantic == semantic)
                return reinterpret_cast<const float*>(stream.data.data + stream.attributes.data[a].offset);
        }
    }
    return nullptr;
}

static bool test_import(const char* name)
{
    Arena input, storage, output;
    Span<uint8_t> bytes = fixture(input, name);
    CHECK(bytes.data);
    ImportedScene scene;
    char error[160] = {};
    CHECK(import_gltf(storage, bytes, scene, error));
    destroy_arena(input);
    ArenaTemp scratch = scratch_begin();
    SDL_memset(arena_allocate(*scratch.arena, 1024 * 1024), 0xdd, 1024 * 1024);
    scratch_end(scratch);
    CHECK(scene.meshes.size == 1 && scene.nodes.size != 0);
    CHECK(scene.meshes.data[0].primitives.data[0].positions.data[3] == 1);
    MeshAsset mesh;
    CHECK(build_mesh_asset(output, scene.meshes.data[0], mesh, error));
    CHECK(attribute(mesh, 0, VertexSemantic::Position) != scene.meshes.data[0].primitives.data[0].positions.data);
    CHECK(mesh.index_type == MeshIndexType::UInt16);
    CHECK(mesh.primitives.data[0].vertex_count == 3);
    CHECK(attribute(mesh, 0, VertexSemantic::Position)[3] == 1 && attribute(mesh, 0, VertexSemantic::Position)[7] == 1);
    CHECK(reinterpret_cast<const uint16_t*>(mesh.indices.data)[0] == 0 && reinterpret_cast<const uint16_t*>(mesh.indices.data)[1] == 1);
    CHECK(reinterpret_cast<const uint16_t*>(mesh.indices.data)[2] == 2);
    CHECK(mesh.bounds.lowerBound.x == 0 && mesh.bounds.upperBound.x == 1 && mesh.bounds.upperBound.y == 1 && mesh.bounds.upperBound.z == 0);
    CHECK(mesh.primitives.data[0].bounds.upperBound.x == 1 && mesh.primitives.data[0].bounds.upperBound.y == 1);
    CHECK(!attribute(mesh, 0, VertexSemantic::Tangent));
    if (SDL_strcmp(name, "multiple.json") == 0) {
        CHECK(scene.materials.size == 2 && scene.nodes.size == 2);
        CHECK(scene.meshes.data[0].primitives.data[0].material == 1 && scene.meshes.data[0].primitives.data[1].material == 0);
        CHECK(mesh.primitives.size == 2 && mesh.indices.size == 12 && mesh.streams.size == 4);
        CHECK(mesh.primitives.data[1].first_index == 3 && mesh.primitives.data[1].index_count == 3);
        CHECK(mesh.primitives.data[1].stream_count == 1 && mesh.primitives.data[1].bounds.upperBound.y == 1);
        CHECK(!attribute(mesh, 1, VertexSemantic::Normal) && !attribute(mesh, 1, VertexSemantic::TexCoord0));
        CHECK(reinterpret_cast<const uint16_t*>(mesh.indices.data)[3] == 0);
        CHECK(scene.nodes.data[1].world[14] == 5);
    } else {
        CHECK(mesh.primitives.size == 1 && mesh.indices.size == 6 && mesh.streams.size == 3);
    }
    if (SDL_strcmp(name, "transform.json") == 0) {
        CHECK(SDL_fabsf(attribute(mesh, 0, VertexSemantic::Normal)[0] - 0.70710678118f) < 0.00001f);
        CHECK(SDL_fabsf(attribute(mesh, 0, VertexSemantic::Normal)[1] - 0.70710678118f) < 0.00001f);
        const float* world = scene.nodes.data[0].world;
        CHECK(world[12] == 2 && world[13] == 3 && world[14] == 4);
        CHECK(SDL_fabsf(world[1] + 2) < 0.00001f && SDL_fabsf(world[4] + 3) < 0.00001f);
    } else {
        CHECK(attribute(mesh, 0, VertexSemantic::Normal)[2] == 1);
    }
    CHECK(attribute(mesh, 0, VertexSemantic::TexCoord0)[2] == 1 && attribute(mesh, 0, VertexSemantic::TexCoord0)[5] == 1);
    const_cast<float*>(scene.meshes.data[0].primitives.data[0].positions.data)[3] = 99;
    destroy_arena(storage);
    CHECK(attribute(mesh, 0, VertexSemantic::Position)[3] == 1);
    CHECK(attribute(mesh, 0, VertexSemantic::TexCoord0)[5] == 1);
    destroy_arena(output);
    return true;
}

static bool test_bad_imports()
{
    const char* from[] = {"\"count\": 3", "\"count\": 3", "\"byteOffset\": 0", "\"byteLength\": 104", "\"bufferView\": 0",
                          "\"mesh\": 0", "\"indices\": 3", "\"byteLength\": 104"};
    const char* to[] = {"\"count\": 0", "\"count\": 18446744073709551615", "\"byteOffset\": 3", "\"byteLength\": 1",
                        ("\"bufferView\": 0, \"sparse\": {\"count\": 1, \"indices\": {\"bufferView\": 3, \"componentType\": 5123},"
                         " \"values\": {\"bufferView\": 0}}"),
                        "\"mesh\": 0, \"children\": [0]", "\"indices\": 3, \"mode\": 1", "\"byteLength\": 104, \"uri\": \"external.bin\""};
    for (uint32_t i = 0; i < sizeof(from) / sizeof(from[0]); ++i) {
        Arena input, storage;
        Span<uint8_t> bytes = fixture(input, "triangle.json", 0, from[i], to[i]);
        CHECK(bytes.data);
        ImportedScene scene;
        char error[160] = {};
        CHECK(!import_gltf(storage, bytes, scene, error) && error[0]);
        destroy_arena(input);
        destroy_arena(storage);
    }
    Arena input, storage;
    Span<uint8_t> bytes = fixture(input, "triangle.json");
    ImportedScene scene;
    char error[160] = {};
    for (size_t size = 0; size < 28; ++size)
        CHECK(!import_gltf(storage, {bytes.data, size}, scene, error));
    uint8_t* changed = const_cast<uint8_t*>(bytes.data);
    changed[bytes.size - 8] = 99;
    CHECK(!import_gltf(storage, bytes, scene, error));
    destroy_arena(input);
    destroy_arena(storage);
    return true;
}

static bool test_encodings()
{
    Arena input, storage;
    Span<uint8_t> bytes = fixture(input, "triangle.json", 0, "\"bufferView\": 2, \"componentType\": 5126",
                                  "\"bufferView\": 2, \"componentType\": 5121, \"normalized\": true");
    CHECK(bytes.data);
    uint8_t* bin = const_cast<uint8_t*>(bytes.data) + bytes.size - 104;
    const uint8_t uv[] = {0, 255, 255, 0, 128, 64};
    SDL_memcpy(bin + 72, uv, sizeof(uv));
    ImportedScene scene;
    char error[160] = {};
    bool imported = import_gltf(storage, bytes, scene, error);
    if (!imported)
        SDL_Log("Encoding import: %s", error);
    CHECK(imported);
    CHECK(scene.meshes.data[0].primitives.data[0].texcoords.data[1] == 1 && scene.meshes.data[0].primitives.data[0].texcoords.data[2] == 1);
    CHECK(SDL_fabsf(scene.meshes.data[0].primitives.data[0].texcoords.data[4] - 128.0f / 255) < 0.00001f);
    destroy_arena(storage);
    destroy_arena(input);
    bytes = fixture(input, "triangle.json", 0, "\"bufferView\": 3, \"componentType\": 5123", "\"bufferView\": 3, \"componentType\": 5121");
    CHECK(bytes.data);
    bin = const_cast<uint8_t*>(bytes.data) + bytes.size - 104;
    bin[96] = 2; bin[97] = 0; bin[98] = 1;
    CHECK(import_gltf(storage, bytes, scene, error));
    CHECK(scene.meshes.data[0].primitives.data[0].indices.data[0] == 2 && scene.meshes.data[0].primitives.data[0].indices.data[1] == 0 && scene.meshes.data[0].primitives.data[0].indices.data[2] == 1);
    destroy_arena(storage);
    destroy_arena(input);
    bytes = fixture(input, "triangle.json", 0, "\"mesh\": 0", "\"mesh\": 0, \"scale\": [1, 0, 1]");
    CHECK(import_gltf(storage, bytes, scene, error));
    MeshAsset mesh;
    CHECK(build_mesh_asset(storage, scene.meshes.data[0], mesh, error));
    destroy_arena(storage);
    destroy_arena(input);
    bytes = fixture(input, "triangle.json", 0, "\"mesh\": 0", "\"mesh\": 0, \"rotation\": [0, 0, 0, 0]");
    CHECK(!import_gltf(storage, bytes, scene, error));
    destroy_arena(storage);
    destroy_arena(input);
    return true;
}

static bool test_mesh_layouts()
{
    Arena input, storage, output;
    char error[160] = {};
    ImportedScene scene;
    MeshAsset mesh;
    CHECK(import_gltf(storage, fixture(input, "tangent.json"), scene, error));
    CHECK(scene.meshes.data[0].primitives.data[0].tangents.size == 12);
    CHECK(build_mesh_asset(output, scene.meshes.data[0], mesh, error));
    destroy_arena(input);
    destroy_arena(storage);
    CHECK(mesh.streams.size == 4 && mesh.streams.data[2].stride == 16);
    CHECK(mesh.streams.data[2].attributes.data[0].format == VertexFormat::Float4);
    CHECK(attribute(mesh, 0, VertexSemantic::Tangent)[0] == 1 && attribute(mesh, 0, VertexSemantic::Tangent)[3] == -1);
    destroy_arena(output);

    CHECK(import_gltf(storage, fixture(input, "index32.json"), scene, error));
    CHECK(scene.meshes.data[0].primitives.data[0].indices.data[2] == 2);
    CHECK(build_mesh_asset(output, scene.meshes.data[0], mesh, error));
    CHECK(mesh.index_type == MeshIndexType::UInt16 && mesh.indices.size == 6);
    destroy_arena(input);
    destroy_arena(storage);
    destroy_arena(output);

    CHECK(import_gltf(storage, fixture(input, "meshes.json", 0, "\"Body\"", "\"\\u0042ody\""), scene, error));
    destroy_arena(input);
    CHECK(scene.meshes.size == 2 && scene.nodes.size == 2 && scene.nodes.data[1].mesh == 1);
    CHECK(SDL_strcmp(scene.meshes.data[0].name, "Body") == 0 && SDL_strcmp(scene.meshes.data[1].name, "Eyes") == 0);
    CHECK(SDL_strcmp(scene.materials.data[1], "Eye") == 0 && scene.meshes.data[1].primitives.data[0].material == 1);
    const ImportedPrimitive& positions_only = scene.meshes.data[1].primitives.data[0];
    CHECK(!positions_only.normals.data && !positions_only.tangents.data && !positions_only.texcoords.data);
    CHECK(build_mesh_asset(output, scene.meshes.data[1], mesh, error));
    destroy_arena(input);
    destroy_arena(storage);
    CHECK(mesh.streams.size == 1 && mesh.streams.data[0].data.size == 36 && mesh.streams.data[0].stride == 12);
    CHECK(mesh.streams.data[0].attributes.size == 1 && mesh.streams.data[0].attributes.data[0].semantic == VertexSemantic::Position);
    CHECK(!attribute(mesh, 0, VertexSemantic::Normal) && !attribute(mesh, 0, VertexSemantic::TexCoord0));
    CHECK(attribute(mesh, 0, VertexSemantic::Position)[3] == 1);
    destroy_arena(output);

    float* positions = arena_allocate<float>(input, 65537 * 3);
    SDL_memset(positions, 0, 65537 * 3 * sizeof(float));
    positions[65536 * 3] = 9;
    uint32_t indices[] = {0, 65535, 65536};
    ImportedPrimitive primitive = {.positions = {positions, 65537 * 3}, .indices = indices};
    ImportedMesh large = {.primitives = {&primitive, 1}};
    CHECK(build_mesh_asset(output, large, mesh, error));
    destroy_arena(input);
    CHECK(mesh.index_type == MeshIndexType::UInt32 && mesh.indices.size == 12 && mesh.bounds.upperBound.x == 9);
    CHECK(reinterpret_cast<const uint32_t*>(mesh.indices.data)[2] == 65536);
    CHECK(attribute(mesh, 0, VertexSemantic::Position)[65536 * 3] == 9);
    destroy_arena(output);
    float triangle[] = {0, 0, 0, 1, 0, 0, 0, 1, 0};
    uint32_t bad_indices[] = {0, 1, 3};
    primitive = {.positions = triangle, .indices = bad_indices};
    CHECK(!build_mesh_asset(output, {.primitives = {&primitive, 1}}, mesh, error));
    return true;
}

struct TestEvents
{
    uint32_t file_loads = 0;
    uint32_t imports = 0;
    uint32_t builds = 0;
};

static bool pump(AssetFactory& factory, TestEvents& counts)
{
    uint64_t deadline = SDL_GetTicks() + 10000;
    for (;;) {
        ArtifactEvent event;
        while (command_next_event(factory.artifacts->commands, event)) {
            if (event.kind == ArtifactKind::File)
                ++counts.file_loads;
            if (event.kind == ArtifactKind::ImportGLTF) {
                ++counts.imports;
                counts.builds += event.result.state == ArtifactState::Ready && !event.cached;
            }
            CHECK(assets_process_event(factory, event));
        }
        assets_tick(factory);
        bool pending = false;
        for (uint32_t i = 0; i < factory.count; ++i)
            pending |= factory.slots[i].phase != AssetPhase::None;
        if (!pending)
            break;
        CHECK(SDL_GetTicks() < deadline);
        SDL_Delay(1);
    }
    return true;
}

static bool next_event(ArtifactCache& cache, ArtifactEvent& event)
{
    uint64_t deadline = SDL_GetTicks() + 10000;
    while (!command_next_event(cache.commands, event)) {
        CHECK(SDL_GetTicks() < deadline);
        SDL_Delay(1);
    }
    return true;
}

static bool test_factory()
{
    Servers servers;
    CHECK(create_servers(servers, {.thread_count = 3, .command_buffers = 2, .commands_per_buffer = 4}));
    char root[1024], path[1200], alias_path[1200], bad_path[1200];
    SDL_snprintf(root, sizeof(root), "%s/assets-%llu", ORBIT_TEST_DIRECTORY, static_cast<unsigned long long>(SDL_GetTicksNS()));
    CHECK(SDL_CreateDirectory(root));
    SDL_snprintf(path, sizeof(path), "%s/triangle.glb", root);
    SDL_snprintf(alias_path, sizeof(alias_path), "%s/alias.glb", root);
    SDL_snprintf(bad_path, sizeof(bad_path), "%s/bad.glb", root);
    Arena fixtures;
    Span<uint8_t> original = fixture(fixtures, "triangle.json");
    CHECK(write_bytes(path, original) && write_bytes(alias_path, original));
    AssetFactory factory;
    CHECK(create_asset_factory(factory, servers.artifacts, root));
    AssetHandle<MeshAsset> handle = asset_load(factory, "triangle.glb");
    CHECK(asset_valid(factory, handle));
    CHECK(handle == asset_load(factory, "./models/../triangle.glb"));
    CHECK(factory.count == 1 && asset_status(factory, handle).id == asset_id("triangle.glb"));
    CHECK(handle == asset_find(factory, asset_id("triangle.glb")) && !asset_valid(factory, asset_find(factory, 0)));
    CHECK(!asset_valid(factory, {.index = handle.index, .generation = handle.generation + 1}));
    CHECK(!asset_valid(factory, asset_load(factory, "../outside.glb")));
    AssetHandle<MeshAsset> alias = asset_load(factory, "alias.glb");
    CHECK(alias != handle && asset_status(factory, alias).id != asset_status(factory, handle).id);
    CHECK(asset_status(factory, handle).state == AssetState::Loading && !asset_get(factory, handle));
    TestEvents counts;
    CHECK(pump(factory, counts));
    CHECK(counts.file_loads == 2 && counts.imports == 2 && counts.builds == 1);
    CHECK(asset_status(factory, handle).state == AssetState::Ready && asset_status(factory, handle).generation == 1);
    const MeshAsset* old = asset_get(factory, handle);
    CHECK(old && old == asset_get(factory, alias) && attribute(*old, 0, VertexSemantic::Position)[3] == 1);

    ArtifactKey imported = {.source = content_hash(original), .kind = ArtifactKind::ImportGLTF, .version = gltf_import_version};
    ArtifactCommandBuffer* commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    artifact_request(*commands, imported);
    artifact_request(*commands, {.source = imported.source, .kind = ArtifactKind::ImportGLTF, .version = gltf_import_version + 1});
    commands_submit(*commands);
    ArtifactEvent event;
    CHECK(next_event(servers.artifacts, event) && event.cached && event.result.meshes.data == old && event.result.scene);
    CHECK(!assets_process_event(factory, event));
    CHECK(next_event(servers.artifacts, event) && event.result.error == ArtifactError::Unsupported);

    Span<uint8_t> replacement = fixture(fixtures, "triangle.json", 1);
    CHECK(write_bytes(path, replacement));
    asset_reload(factory, handle);
    CHECK(asset_get(factory, handle) == old && asset_status(factory, handle).loading);
    assets_tick(factory);
    CHECK(next_event(servers.artifacts, event) && event.kind == ArtifactKind::File && event.result.ready_generation == 2);
    CHECK(assets_process_event(factory, event));
    ContentHash source = event.result.content;
    ContentHash import_key = artifact_hash({.source = source, .kind = ArtifactKind::ImportGLTF, .version = gltf_import_version});
    SDL_RWLock* gate = servers.content.stripes[source.bytes[0] % 16].lock;
    SDL_LockRWLockForWriting(gate);
    assets_tick(factory);
    uint64_t deadline = SDL_GetTicks() + 10000;
    ArtifactView building;
    do {
        building = artifact_get(servers.artifacts, import_key);
        if (building.state == ArtifactState::Building)
            break;
        SDL_Delay(1);
    } while (SDL_GetTicks() < deadline);
    bool preserved = asset_get(factory, handle) == old && asset_status(factory, handle).state == AssetState::Ready;
    SDL_UnlockRWLock(gate);
    CHECK(building.state == ArtifactState::Building && preserved);
    CHECK(next_event(servers.artifacts, event) && event.result.meshes.data);
    CHECK(asset_get(factory, handle) == old);
    CHECK(assets_process_event(factory, event));
    const MeshAsset* fresh = asset_get(factory, handle);
    CHECK(fresh != old && attribute(*fresh, 0, VertexSemantic::Position)[3] == 2);
    CHECK(asset_status(factory, handle).generation == 2 && handle == asset_load(factory, "triangle.glb"));
    CHECK(asset_get(factory, alias) == old && attribute(*old, 0, VertexSemantic::Position)[3] == 1);

    const uint8_t broken[] = {0, 1, 2, 3, 4, 5};
    CHECK(write_bytes(path, broken) && write_bytes(bad_path, broken));
    asset_reload(factory, handle);
    AssetHandle<MeshAsset> bad = asset_load(factory, "bad.glb");
    AssetHandle<MeshAsset> missing = asset_load(factory, "missing.glb");
    CHECK(pump(factory, counts));
    CHECK(asset_status(factory, bad).state == AssetState::Failed && !asset_get(factory, bad));
    CHECK(asset_status(factory, missing).state == AssetState::Failed && asset_status(factory, missing).error == AssetError::Source);
    CHECK(asset_get(factory, handle) == fresh && asset_status(factory, handle).generation == 2);
    CHECK(asset_status(factory, handle).state == AssetState::Ready && asset_status(factory, handle).error == AssetError::Import);
    CHECK(write_bytes(path, replacement));
    asset_reload(factory, handle);
    CHECK(pump(factory, counts) && asset_status(factory, handle).error == AssetError::None);
    CHECK(asset_get(factory, handle) == fresh && asset_status(factory, handle).generation == 4);

    for (uint32_t i = 0; i < 130; ++i) {
        char name[64];
        SDL_snprintf(name, sizeof(name), "missing-%u.glb", i);
        CHECK(asset_valid(factory, asset_load(factory, name)));
    }
    CHECK(factory.capacity > 128 && handle == asset_load(factory, "triangle.glb") && asset_get(factory, handle) == fresh);
    // Only one buffer is available; queued asset requests survive backpressure instead of disappearing.
    ArtifactCommandBuffer* reserved = commands_begin(servers.artifacts.commands);
    CHECK(reserved);
    assets_tick(factory);
    bool queued = false;
    for (uint32_t i = 0; i < factory.count; ++i)
        queued |= factory.slots[i].phase == AssetPhase::FileQueued;
    CHECK(queued);
    commands_discard(*reserved);
    CHECK(pump(factory, counts));

    // Cache work holds no factory/slot pointers, even when the factory disappears before import completes.
    Span<uint8_t> pending = fixture(fixtures, "triangle.json", 2);
    CHECK(write_bytes(path, pending));
    asset_reload(factory, handle);
    assets_tick(factory);
    CHECK(next_event(servers.artifacts, event) && assets_process_event(factory, event));
    source = event.result.content;
    import_key = artifact_hash({.source = source, .kind = ArtifactKind::ImportGLTF, .version = gltf_import_version});
    gate = servers.content.stripes[source.bytes[0] % 16].lock;
    SDL_LockRWLockForWriting(gate);
    assets_tick(factory);
    deadline = SDL_GetTicks() + 10000;
    do {
        building = artifact_get(servers.artifacts, import_key);
        if (building.state == ArtifactState::Building)
            break;
        SDL_Delay(1);
    } while (SDL_GetTicks() < deadline);
    destroy_asset_factory(factory);
    SDL_UnlockRWLock(gate);
    CHECK(building.state == ArtifactState::Building);
    stop_servers(servers);
    CHECK(next_event(servers.artifacts, event) && event.result.state == ArtifactState::Ready);
    CHECK(attribute(event.result.meshes.data[0], 0, VertexSemantic::Position)[3] == 3);
    destroy_servers(servers);
    destroy_arena(fixtures);
    CHECK(SDL_RemovePath(path) && SDL_RemovePath(alias_path) && SDL_RemovePath(bad_path) && SDL_RemovePath(root));
    return true;
}

static bool test_render_requests()
{
    Servers servers;
    CHECK(create_servers(servers, {.thread_count = 2, .command_buffers = 2}));
    AssetFactory factory;
    CHECK(create_asset_factory(factory, servers.artifacts, ORBIT_TEST_DIRECTORY));
    Arena fixtures;
    CHECK(write_bytes(ORBIT_TEST_DIRECTORY "/render-request.glb", fixture(fixtures, "triangle.json")));
    AssetHandle<MeshAsset> handle = asset_load(factory, "render-request.glb");
    RenderAssetCache cache = {.assets = &factory};
    CHECK(request_render_mesh(cache, asset_id("render-request.glb")) == handle);
    CHECK(cache.entries[handle.index].state == GPUAssetState::Missing && !cache.pending_count);
    TestEvents counts;
    CHECK(pump(factory, counts));
    render_assets_process_changes(cache);
    CHECK(cache.pending_count == 1 && cache.entries[handle.index].state == GPUAssetState::PendingUpload);
    CHECK(!render_mesh(cache, handle));
    CHECK(request_render_mesh(cache, asset_id("render-request.glb")) == handle && cache.pending_count == 1);
    asset_reload(factory, handle);
    CHECK(pump(factory, counts));
    render_assets_process_changes(cache);
    CHECK(cache.pending_count == 1);
    CHECK(write_bytes(ORBIT_TEST_DIRECTORY "/render-request.glb", fixture(fixtures, "triangle.json", 1)));
    asset_reload(factory, handle);
    CHECK(pump(factory, counts));
    render_assets_process_changes(cache);
    CHECK(cache.pending_count == 1 && cache.entries[handle.index].observed == asset_status(factory, handle).source);
    asset_unload(factory, handle);
    CHECK(!asset_valid(factory, handle) && !asset_valid(factory, asset_find(factory, asset_id("render-request.glb"))));
    render_assets_process_changes(cache);
    CHECK(cache.entries[handle.index].state == GPUAssetState::Missing && !render_mesh(cache, handle));
    // Reusing the logical name gets a new handle; a still-queued old request must not upload a loading asset.
    AssetHandle<MeshAsset> next = asset_load(factory, "render-request.glb");
    CHECK(next.index == handle.index && next.generation != handle.generation);
    CHECK(request_render_mesh(cache, asset_id("render-request.glb")) == next);
    GPUTransferQueue transfers;
    prepare_render_assets(cache, transfers);
    CHECK(!transfers.count && !transfers.staging);
    finish_render_assets(cache, transfers);
    CHECK(!cache.pending_count);
    CHECK(pump(factory, counts));
    render_assets_process_changes(cache);
    CHECK(cache.pending_count == 1 && !render_mesh(cache, next));
    asset_reload(factory, next);
    assets_tick(factory);
    asset_unload(factory, next);
    ArtifactEvent event;
    CHECK(next_event(servers.artifacts, event));
    CHECK(!assets_process_event(factory, event));
    CHECK(!asset_valid(factory, next));
    render_assets_process_changes(cache);
    prepare_render_assets(cache, transfers);
    finish_render_assets(cache, transfers);
    CHECK(!cache.pending_count);
    destroy_render_asset_cache(cache);
    destroy_asset_factory(factory);
    destroy_servers(servers);
    destroy_arena(fixtures);
    CHECK(SDL_RemovePath(ORBIT_TEST_DIRECTORY "/render-request.glb"));
    return true;
}

static bool test_subassets()
{
    Servers servers;
    CHECK(create_servers(servers, {.thread_count = 3, .command_buffers = 2, .commands_per_buffer = 4}));
    char root[1024], path[1200];
    SDL_snprintf(root, sizeof(root), "%s/subassets-%llu", ORBIT_TEST_DIRECTORY, static_cast<unsigned long long>(SDL_GetTicksNS()));
    CHECK(SDL_CreateDirectory(root));
    SDL_snprintf(path, sizeof(path), "%s/model.glb", root);
    Arena fixtures;
    CHECK(write_bytes(path, fixture(fixtures, "meshes.json")));
    AssetFactory factory;
    CHECK(create_asset_factory(factory, servers.artifacts, root));
    AssetHandle<MeshAsset> body = asset_load(factory, "model.glb#Body");
    AssetHandle<MeshAsset> eyes = asset_load(factory, "model.glb#Eyes");
    AssetHandle<MeshAsset> ambiguous = asset_load(factory, "model.glb");
    AssetHandle<MeshAsset> missing = asset_load(factory, "model.glb#Missing");
    CHECK(body == asset_load(factory, "x/../model.glb#Body") && body != eyes);
    CHECK(asset_status(factory, body).id == asset_id("model.glb#Body"));
    CHECK(asset_status(factory, body).id != asset_status(factory, eyes).id);
    CHECK(!asset_valid(factory, asset_load(factory, "model.glb#")));
    CHECK(SDL_strcmp(factory.slots[body.index].path, factory.slots[eyes.index].path) == 0);
    TestEvents counts;
    CHECK(pump(factory, counts));
    CHECK(counts.imports == 4 && counts.builds == 1);
    CHECK(asset_status(factory, ambiguous).state == AssetState::Failed && asset_status(factory, missing).state == AssetState::Failed);
    const MeshAsset* old_body = asset_get(factory, body);
    const MeshAsset* old_eyes = asset_get(factory, eyes);
    CHECK(old_body && old_eyes && old_body != old_eyes);
    CHECK(old_body->streams.size == 3 && old_eyes->streams.size == 1);
    CHECK(old_body->primitives.size == 1 && old_eyes->primitives.size == 1);
    CHECK(write_bytes(path, fixture(fixtures, "meshes_reordered.json", 1)));
    asset_reload(factory, body);
    asset_reload(factory, eyes);
    CHECK(asset_get(factory, body) == old_body && asset_get(factory, eyes) == old_eyes);
    CHECK(pump(factory, counts) && counts.builds == 2);
    CHECK(asset_load(factory, "model.glb#Body") == body && asset_load(factory, "model.glb#Eyes") == eyes);
    CHECK(asset_get(factory, body)->streams.size == 3 && asset_get(factory, eyes)->streams.size == 1);
    CHECK(attribute(*asset_get(factory, body), 0, VertexSemantic::Position)[3] == 2);
    CHECK(asset_status(factory, body).generation == 2);
    const MeshAsset* current = asset_get(factory, body);
    CHECK(write_bytes(path, fixture(fixtures, "meshes.json", 2, "\"Eyes\"", "\"Body\"")));
    asset_reload(factory, body);
    CHECK(pump(factory, counts));
    CHECK(asset_get(factory, body) == current && asset_status(factory, body).error == AssetError::Import);
    CHECK(asset_status(factory, body).state == AssetState::Ready && asset_status(factory, body).generation == 2);
    destroy_asset_factory(factory);
    destroy_servers(servers);
    destroy_arena(fixtures);
    CHECK(SDL_RemovePath(path) && SDL_RemovePath(root));
    return true;
}

int main()
{
    if (!initialize_memory() || !SDL_Init(0))
        return 1;
    ThreadContext context;
    if (!set_thread_context(&context))
        return 1;
    if (!test_paths() || !test_import("triangle.json") || !test_import("multiple.json") || !test_import("transform.json") ||
        !test_bad_imports() || !test_encodings() || !test_mesh_layouts() || !test_factory() || !test_render_requests() || !test_subassets())
        return 1;
    destroy_thread_context(context);
    SDL_Quit();
    SDL_Log("Static mesh asset pipeline tests passed");
    return 0;
}
