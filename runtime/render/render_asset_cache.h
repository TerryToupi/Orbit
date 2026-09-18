#ifndef ORBIT_RENDER_ASSET_CACHE_H
#define ORBIT_RENDER_ASSET_CACHE_H

#include <asset_factory.h>
#include <render/gpu_mesh.h>
#include <render/gpu_transfer_queue.h>

enum class GPUAssetState : uint32_t { Missing, PendingUpload, Ready };

struct RenderMeshEntry
{
    GpuMesh mesh = {};
    GpuMesh replacement = {};
    ContentHash observed = {};
    AssetHandle<MeshAsset> asset = {};
    GPUAssetState state = GPUAssetState::Missing; // requested version; mesh retains the ready fallback
    bool queued = false;
};

// Main-thread owned, indexed by factory slot. Borrows factory/device; owns GPU meshes and persistent arrays.
// Resolve draw pointers after requests/change dispatch/preparation; borrow through that frame's submission only.
struct RenderAssetCache
{
    AssetFactory* assets = nullptr;
    SDL_GPUDevice* device = nullptr;
    RenderMeshEntry* entries = nullptr;
    uint32_t* pending = nullptr;
    uint32_t capacity = 0;
    uint32_t pending_count = 0;
};

// AssetID lookup happens when a mesh becomes needed. Retain the returned handle for constant-time draw resolution.
AssetHandle<MeshAsset> request_render_mesh(RenderAssetCache& cache, AssetID id);
void destroy_render_asset_cache(RenderAssetCache& cache);
// Returns the last successfully submitted mesh, including during replacement; null before the first upload.
const GpuMesh* render_mesh(const RenderAssetCache& cache, AssetHandle<MeshAsset> handle);
void render_assets_process_changes(RenderAssetCache& cache);
// Prepare once after asset changes; no asset/cache mutation until finish. Failures log and preserve live meshes.
void prepare_render_assets(RenderAssetCache& cache, GPUTransferQueue& transfers);
// Call after both command buffers were submitted/cancelled, even on frame failure. Publishes for the next frame.
void finish_render_assets(RenderAssetCache& cache, GPUTransferQueue& transfers);

#endif
