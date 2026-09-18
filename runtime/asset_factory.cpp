#include <asset_factory.h>
#include <path.h>
#include <thread_context.h>
#include <SDL3/SDL_error.h>

static void asset_changed(AssetFactory& factory, uint32_t index)
{
    if (!factory.slots[index].changed) {
        factory.slots[index].changed = true;
        factory.changes[factory.change_count++] = index;
    }
}

bool asset_next_change(AssetFactory& factory, AssetHandle<MeshAsset>& handle)
{
    assert(factory.thread == SDL_GetCurrentThreadID());
    if (!factory.change_count)
        return false;
    uint32_t index = factory.changes[--factory.change_count];
    factory.slots[index].changed = false;
    handle = {.index = index, .generation = factory.slots[index].handle_generation};
    return true;
}

void asset_unload(AssetFactory& factory, AssetHandle<MeshAsset> handle)
{
    assert(asset_valid(factory, handle));
    AssetSlot& slot = factory.slots[handle.index];
    slot.current = nullptr;
    slot.source = {};
    slot.generation = 0;
    slot.request = 0;
    slot.phase = AssetPhase::None;
    slot.state = AssetState::Unloaded;
    slot.error = AssetError::None;
    slot.reload_again = false;
    slot.message[0] = 0;
    ++slot.handle_generation;
    asset_changed(factory, handle.index);
}

AssetID asset_id(const char* normalized_name)
{
    assert(normalized_name);
    uint64_t hash = 14695981039346656037ull;
    for (const uint8_t* p = reinterpret_cast<const uint8_t*>(normalized_name); *p; ++p)
        hash = (hash ^ *p) * 1099511628211ull;
    return hash ? hash : 1;
}

bool create_asset_factory(AssetFactory& factory, ArtifactCache& artifacts, const char* root)
{
    factory.root = path_absolute(factory.storage, root);
    if (!factory.root) {
        destroy_arena(factory.storage);
        return false;
    }
    factory.artifacts = &artifacts;
    factory.thread = SDL_GetCurrentThreadID();
    factory.capacity = 128;
    factory.slots = static_cast<AssetSlot*>(SDL_malloc(sizeof(AssetSlot) * factory.capacity));
    factory.index = static_cast<AssetIndex*>(SDL_malloc(sizeof(AssetIndex) * factory.capacity));
    factory.changes = static_cast<uint32_t*>(SDL_malloc(sizeof(uint32_t) * factory.capacity));
    return true;
}

void destroy_asset_factory(AssetFactory& factory)
{
    assert(!factory.thread || factory.thread == SDL_GetCurrentThreadID());
    SDL_free(factory.slots);
    SDL_free(factory.index);
    SDL_free(factory.changes);
    destroy_arena(factory.storage);
    factory = {};
}

AssetHandle<MeshAsset> asset_load(AssetFactory& factory, const char* name)
{
    assert(factory.thread == SDL_GetCurrentThreadID());
    assert(name);
    ArenaTemp scratch = scratch_begin({&factory.storage});
    const char* selector = SDL_strchr(name, '#');
    char* source = arena_allocate<char>(*scratch.arena, (selector ? size_t(selector - name) : SDL_strlen(name)) + 1);
    SDL_strlcpy(source, name, (selector ? size_t(selector - name) : SDL_strlen(name)) + 1);
    const char* normalized = path_normalize(*scratch.arena, source);
    if (!normalized || normalized[0] == '/' || SDL_strchr(normalized, ':') || SDL_strcmp(normalized, ".") == 0) {
        SDL_SetError("Asset names must be nonempty root-relative paths");
        scratch_end(scratch);
        return {};
    }
    const char* physical = normalized;
    if (selector) {
        if (!selector[1]) {
            SDL_SetError("Mesh selector must not be empty");
            scratch_end(scratch);
            return {};
        }
        char* identity = arena_allocate<char>(*scratch.arena, SDL_strlen(normalized) + SDL_strlen(selector) + 1);
        SDL_snprintf(identity, SDL_strlen(normalized) + SDL_strlen(selector) + 1, "%s%s", normalized, selector);
        normalized = identity;
    }
    AssetID id = asset_id(normalized);
    uint32_t first = 0, last = factory.count;
    while (first < last) {
        uint32_t mid = first + (last - first) / 2;
        if (factory.index[mid].id < id)
            first = mid + 1;
        else
            last = mid;
    }
    if (first < factory.count && factory.index[first].id == id) {
        uint32_t slot = factory.index[first].slot;
        bool same = SDL_strcmp(factory.slots[slot].name, normalized) == 0;
        scratch_end(scratch);
        if (!same) {
            SDL_SetError("AssetID collision");
            return {};
        }
        if (factory.slots[slot].state == AssetState::Unloaded) {
            factory.slots[slot].state = AssetState::Loading;
            factory.slots[slot].phase = AssetPhase::FileQueued;
        }
        return {.index = slot, .generation = factory.slots[slot].handle_generation};
    }
    if (factory.count == factory.capacity) {
        factory.capacity += 128;
        factory.slots = static_cast<AssetSlot*>(SDL_realloc(factory.slots, sizeof(AssetSlot) * factory.capacity));
        factory.index = static_cast<AssetIndex*>(SDL_realloc(factory.index, sizeof(AssetIndex) * factory.capacity));
        factory.changes = static_cast<uint32_t*>(SDL_realloc(factory.changes, sizeof(uint32_t) * factory.capacity));
    }
    char* copied = arena_allocate<char>(factory.storage, SDL_strlen(normalized) + 1);
    SDL_memcpy(copied, normalized, SDL_strlen(normalized) + 1);
    uint32_t slot = factory.count++;
    factory.slots[slot] = {.id = id, .name = copied, .path = path_join(factory.storage, factory.root, physical),
                          .mesh_name = selector ? copied + SDL_strlen(physical) + 1 : nullptr};
    SDL_memmove(factory.index + first + 1, factory.index + first, (factory.count - first - 1) * sizeof(AssetIndex));
    factory.index[first] = {.id = id, .slot = slot};
    scratch_end(scratch);
    return {.index = slot, .generation = factory.slots[slot].handle_generation};
}

AssetHandle<MeshAsset> asset_find(const AssetFactory& factory, AssetID id)
{
    assert(factory.thread == SDL_GetCurrentThreadID());
    uint32_t first = 0, last = factory.count;
    while (first < last) {
        uint32_t mid = first + (last - first) / 2;
        if (factory.index[mid].id < id)
            first = mid + 1;
        else
            last = mid;
    }
    if (first == factory.count || factory.index[first].id != id)
        return {};
    uint32_t slot = factory.index[first].slot;
    if (factory.slots[slot].state == AssetState::Unloaded)
        return {};
    return {.index = slot, .generation = factory.slots[slot].handle_generation};
}

bool asset_valid(const AssetFactory& factory, AssetHandle<MeshAsset> handle)
{
    assert(factory.thread == SDL_GetCurrentThreadID());
    return handle.index < factory.count && factory.slots[handle.index].handle_generation == handle.generation &&
           factory.slots[handle.index].state != AssetState::Unloaded;
}

AssetStatus asset_status(const AssetFactory& factory, AssetHandle<MeshAsset> handle)
{
    assert(asset_valid(factory, handle));
    const AssetSlot& slot = factory.slots[handle.index];
    return {.id = slot.id, .generation = slot.generation, .source = slot.source,
            .state = slot.state, .error = slot.error, .loading = slot.phase != AssetPhase::None};
}

const MeshAsset* asset_get(const AssetFactory& factory, AssetHandle<MeshAsset> handle)
{
    assert(asset_valid(factory, handle));
    return factory.slots[handle.index].current;
}

void asset_reload(AssetFactory& factory, AssetHandle<MeshAsset> handle)
{
    assert(asset_valid(factory, handle));
    AssetSlot& slot = factory.slots[handle.index];
    if (slot.phase != AssetPhase::None) {
        slot.reload_again = true;
        return;
    }
    slot.phase = AssetPhase::FileQueued;
    slot.error = AssetError::None;
    slot.message[0] = 0;
    if (!slot.current)
        slot.state = AssetState::Loading;
}

void assets_tick(AssetFactory& factory)
{
    assert(factory.thread == SDL_GetCurrentThreadID());
    if (!factory.artifacts->commands.signal->accepting)
        return;
    ArtifactCommandBuffer* commands = nullptr;
    for (uint32_t i = 0; i < factory.count; ++i) {
        AssetSlot& slot = factory.slots[i];
        if (slot.phase != AssetPhase::FileQueued && slot.phase != AssetPhase::ImportQueued)
            continue;
        if (commands && commands->count == commands->owner->command_limit) {
            commands_submit(*commands);
            commands = nullptr;
        }
        if (!commands) {
            commands = commands_begin(factory.artifacts->commands);
            if (!commands)
                break;
        }
        if (slot.phase == AssetPhase::FileQueued) {
            slot.request = file_load(*commands, slot.path);
            slot.phase = AssetPhase::FilePending;
        } else {
            slot.request = artifact_request(*commands, {.source = slot.pending_source, .kind = ArtifactKind::ImportGLTF, .version = gltf_import_version});
            slot.phase = AssetPhase::ImportPending;
        }
    }
    if (commands)
        commands_submit(*commands);
}

bool assets_process_event(AssetFactory& factory, const ArtifactEvent& event)
{
    assert(factory.thread == SDL_GetCurrentThreadID());
    for (uint32_t i = 0; i < factory.count; ++i) {
        AssetSlot& slot = factory.slots[i];
        if (slot.request != event.request || (slot.phase != AssetPhase::FilePending && slot.phase != AssetPhase::ImportPending))
            continue;
        if (event.result.state != ArtifactState::Ready) {
            slot.error = slot.phase == AssetPhase::FilePending ? AssetError::Source : AssetError::Import;
            SDL_strlcpy(slot.message, event.error[0] ? event.error : "Asset source or import failed", sizeof(slot.message));
            slot.state = slot.current ? AssetState::Ready : AssetState::Failed;
            slot.phase = AssetPhase::None;
        } else if (slot.phase == AssetPhase::FilePending) {
            slot.pending_generation = event.result.ready_generation;
            slot.pending_source = event.result.content;
            if (slot.current && slot.source == slot.pending_source) {
                slot.generation = slot.pending_generation;
                asset_changed(factory, i);
                slot.phase = AssetPhase::None;
                slot.error = AssetError::None;
                slot.message[0] = 0;
            } else {
                slot.phase = AssetPhase::ImportQueued;
            }
        } else {
            assert(event.result.scene && event.result.meshes.size == event.result.scene->meshes.size);
            const MeshAsset* selected = nullptr;
            if (!slot.mesh_name && event.result.meshes.size == 1)
                selected = event.result.meshes.data;
            if (slot.mesh_name) {
                for (size_t m = 0; m < event.result.meshes.size; ++m) {
                    const char* name = event.result.scene->meshes.data[m].name;
                    if (name && SDL_strcmp(name, slot.mesh_name) == 0) {
                        if (selected) {
                            selected = nullptr;
                            break;
                        }
                        selected = event.result.meshes.data + m;
                    }
                }
            }
            if (selected) {
                slot.current = selected;
                asset_changed(factory, i);
                slot.source = slot.pending_source;
                slot.generation = slot.pending_generation;
                slot.state = AssetState::Ready;
                slot.error = AssetError::None;
                slot.message[0] = 0;
            } else {
                slot.state = slot.current ? AssetState::Ready : AssetState::Failed;
                slot.error = AssetError::Import;
                SDL_strlcpy(slot.message, "Mesh selection is missing or ambiguous; use a unique source mesh name", sizeof(slot.message));
            }
            slot.phase = AssetPhase::None;
        }
        if (slot.phase == AssetPhase::None && slot.reload_again) {
            slot.reload_again = false;
            slot.phase = AssetPhase::FileQueued;
            if (!slot.current)
                slot.state = AssetState::Loading;
        }
        return true;
    }
    return false;
}
