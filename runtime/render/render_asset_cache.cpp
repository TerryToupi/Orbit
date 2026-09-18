#include <render/render_asset_cache.h>
#include <SDL3/SDL_log.h>

static void update_render_mesh(RenderAssetCache& cache, AssetHandle<MeshAsset> handle)
{
    RenderMeshEntry& entry = cache.entries[handle.index];
    AssetStatus status = asset_status(*cache.assets, handle);
    if (status.state != AssetState::Ready || status.source == entry.observed)
        return;
    entry.observed = status.source;
    entry.state = GPUAssetState::PendingUpload;
    if (!entry.queued) {
        entry.queued = true;
        cache.pending[cache.pending_count++] = handle.index;
    }
}

AssetHandle<MeshAsset> request_render_mesh(RenderAssetCache& cache, AssetID id)
{
    AssetHandle<MeshAsset> handle = asset_find(*cache.assets, id);
    if (!asset_valid(*cache.assets, handle))
        return {};
    if (handle.index >= cache.capacity) {
        uint32_t capacity = (handle.index / 128 + 1) * 128;
        cache.entries = static_cast<RenderMeshEntry*>(SDL_realloc(cache.entries, sizeof(RenderMeshEntry) * capacity));
        cache.pending = static_cast<uint32_t*>(SDL_realloc(cache.pending, sizeof(uint32_t) * capacity));
        for (uint32_t i = cache.capacity; i < capacity; ++i)
            cache.entries[i] = {};
        cache.capacity = capacity;
    }
    RenderMeshEntry& entry = cache.entries[handle.index];
    if (entry.asset != handle) {
        destroy_gpu_mesh(cache.device, entry.mesh);
        destroy_gpu_mesh(cache.device, entry.replacement);
        entry.observed = {};
        entry.asset = handle;
        entry.state = GPUAssetState::Missing;
    }
    update_render_mesh(cache, handle);
    return handle;
}

void destroy_render_asset_cache(RenderAssetCache& cache)
{
    for (uint32_t i = 0; i < cache.capacity; ++i) {
        destroy_gpu_mesh(cache.device, cache.entries[i].mesh);
        destroy_gpu_mesh(cache.device, cache.entries[i].replacement);
    }
    SDL_free(cache.entries);
    SDL_free(cache.pending);
    cache = {};
}

const GpuMesh* render_mesh(const RenderAssetCache& cache, AssetHandle<MeshAsset> handle)
{
    if (handle.index >= cache.capacity)
        return nullptr;
    const RenderMeshEntry& entry = cache.entries[handle.index];
    return entry.asset == handle && entry.mesh.vertices ? &entry.mesh : nullptr;
}

void render_assets_process_changes(RenderAssetCache& cache)
{
    AssetHandle<MeshAsset> handle;
    while (asset_next_change(*cache.assets, handle)) {
        if (handle.index >= cache.capacity)
            continue;
        RenderMeshEntry& entry = cache.entries[handle.index];
        if (entry.asset != handle || !asset_valid(*cache.assets, handle)) {
            destroy_gpu_mesh(cache.device, entry.mesh);
            destroy_gpu_mesh(cache.device, entry.replacement);
            entry.asset = {};
            entry.observed = {};
            entry.state = GPUAssetState::Missing;
        } else {
            update_render_mesh(cache, handle);
        }
    }
}

void prepare_render_assets(RenderAssetCache& cache, GPUTransferQueue& transfers)
{
    assert(!transfers.count && !transfers.mapped);
    size_t bytes = 0;
    uint32_t upload_count = 0;
    for (uint32_t i = 0; i < cache.pending_count; ++i) {
        RenderMeshEntry& entry = cache.entries[cache.pending[i]];
        if (entry.state != GPUAssetState::PendingUpload || !asset_valid(*cache.assets, entry.asset))
            continue;
        const MeshAsset* source = asset_get(*cache.assets, entry.asset);
        assert(source && !entry.replacement.vertices);
        if (!create_gpu_mesh(cache.device, *source, entry.replacement)) {
            SDL_Log("Mesh GPU creation failed: %s", SDL_GetError());
            continue;
        }
        bytes += entry.replacement.vertex_bytes + ((source->indices.size + 3) & ~size_t(3));
        upload_count += 2;
    }
    if (!begin_gpu_uploads(transfers, cache.device, bytes, upload_count)) {
        SDL_Log("Mesh staging preparation failed: %s", SDL_GetError());
        for (uint32_t i = 0; i < cache.pending_count; ++i)
            destroy_gpu_mesh(cache.device, cache.entries[cache.pending[i]].replacement);
        return;
    }
    for (uint32_t i = 0; i < cache.pending_count; ++i) {
        RenderMeshEntry& entry = cache.entries[cache.pending[i]];
        if (!entry.replacement.vertices)
            continue;
        const MeshAsset& source = *asset_get(*cache.assets, entry.asset);
        size_t used = transfers.used;
        uint8_t* vertices = queue_gpu_upload(transfers, entry.replacement.vertices, entry.replacement.vertex_bytes);
        uint8_t* indices = queue_gpu_upload(transfers, entry.replacement.indices, source.indices.size);
        if (!pack_gpu_mesh(source, entry.replacement, vertices, indices)) {
            SDL_Log("Mesh upload preparation failed: %s", SDL_GetError());
            transfers.count -= 2;
            transfers.used = used;
            destroy_gpu_mesh(cache.device, entry.replacement);
        }
    }
}

void finish_render_assets(RenderAssetCache& cache, GPUTransferQueue& transfers)
{
    for (uint32_t i = 0; i < cache.pending_count; ++i) {
        RenderMeshEntry& entry = cache.entries[cache.pending[i]];
        if (transfers.submitted && entry.replacement.vertices) {
            destroy_gpu_mesh(cache.device, entry.mesh);
            entry.mesh = entry.replacement;
            entry.replacement = {};
        } else {
            destroy_gpu_mesh(cache.device, entry.replacement);
        }
        entry.state = entry.mesh.vertices ? GPUAssetState::Ready : GPUAssetState::Missing;
        entry.queued = false;
    }
    cache.pending_count = 0;
    reset_gpu_transfers(transfers, cache.device);
}
