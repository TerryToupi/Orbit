#ifndef ORBIT_ASSET_FACTORY_H
#define ORBIT_ASSET_FACTORY_H

#include <artifact_cache.h>
#include <mesh.h>
#include <SDL3/SDL_thread.h>

using AssetID = uint64_t;

template<typename T>
struct AssetHandle
{
    uint32_t index = UINT32_MAX;
    uint32_t generation = 0;
    bool operator==(const AssetHandle&) const = default;
};

enum class AssetState : uint32_t { Loading, Ready, Failed };
enum class AssetError : uint32_t { None, Source, Import };
enum class AssetPhase : uint32_t { None, FileQueued, FilePending, ImportQueued, ImportPending };

struct AssetStatus
{
    AssetID id = 0;
    uint64_t generation = 0;
    AssetState state = AssetState::Loading;
    AssetError error = AssetError::None;
    bool loading = false;
};

struct AssetSlot
{
    AssetID id = 0;
    const char* name = nullptr;
    const char* path = nullptr;
    const char* mesh_name = nullptr;
    const MeshAsset* current = nullptr;
    ContentHash source = {};
    ContentHash pending_source = {};
    uint64_t generation = 0;
    uint64_t pending_generation = 0;
    uint64_t request = 0;
    uint32_t handle_generation = 1;
    AssetState state = AssetState::Loading;
    AssetError error = AssetError::None;
    AssetPhase phase = AssetPhase::FileQueued;
    bool reload_again = false;
    char message[160] = {};
};

struct AssetIndex
{
    AssetID id = 0;
    uint32_t slot = 0;
};

// Main-thread owned. Slots/paths belong to the factory; immutable meshes belong to the artifact cache.
// No slot recycling in V1. Factory/cache must outlive handle use; destroy factory before destroying the server caches.
struct AssetFactory
{
    ArtifactCache* artifacts = nullptr;
    SDL_ThreadID thread = 0;
    Arena storage = {};
    const char* root = nullptr;
    AssetSlot* slots = nullptr;
    AssetIndex* index = nullptr;
    uint32_t count = 0;
    uint32_t capacity = 0;
};

bool create_asset_factory(AssetFactory& factory, ArtifactCache& artifacts, const char* root);
void destroy_asset_factory(AssetFactory& factory);
// FNV-1a 64 of a normalized UTF-8 root-relative name; independent of installation directory. Zero is reserved.
AssetID asset_id(const char* normalized_name);
// No IO. "path.glb#MeshName" selects a unique source mesh name; a bare path requires exactly one mesh.
// '#' is reserved for selection. Invalid logical names/collisions return an invalid handle and set SDL_GetError.
AssetHandle<MeshAsset> asset_load(AssetFactory& factory, const char* name);
AssetHandle<MeshAsset> asset_find(const AssetFactory& factory, AssetID id);
bool asset_valid(const AssetFactory& factory, AssetHandle<MeshAsset> handle);
AssetStatus asset_status(const AssetFactory& factory, AssetHandle<MeshAsset> handle);
// Borrow until the next factory event/reload/destruction; persist the handle, not this pointer. Null until first successful load.
const MeshAsset* asset_get(const AssetFactory& factory, AssetHandle<MeshAsset> handle);
void asset_reload(AssetFactory& factory, AssetHandle<MeshAsset> handle);

// Main-loop plumbing: submit queued batches, and dispatch polled artifact events. False leaves an event for another consumer.
// Backpressure keeps requests queued for a later tick; no blocking wait or private result draining.
void assets_tick(AssetFactory& factory);
bool assets_process_event(AssetFactory& factory, const ArtifactEvent& event);

#endif
