#include <artifact_cache.h>
#include <thread_context.h>
#include <gltf_import.h>
#include <SDL3/SDL_stdinc.h>

ContentHash artifact_hash(const ArtifactKey& key)
{
    uint8_t bytes[72];
    SDL_memcpy(bytes, key.source.bytes, 32);
    SDL_memcpy(bytes + 32, key.parameters.bytes, 32);
    for (uint32_t i = 0; i < 4; ++i) {
        bytes[64 + i] = uint8_t(uint32_t(key.kind) >> (8 * i));
        bytes[68 + i] = uint8_t(key.version >> (8 * i));
    }
    return content_hash({bytes, sizeof(bytes)});
}

static uint32_t artifact_owner(const ContentHash& hash, uint32_t lanes)
{
    return ((uint32_t(hash.bytes[0]) << 24) | (uint32_t(hash.bytes[1]) << 16) | (uint32_t(hash.bytes[2]) << 8) | hash.bytes[3]) % lanes;
}

static uint32_t artifact_position(const ArtifactLane& lane, const ContentHash& hash)
{
    uint32_t first = 0, last = lane.count;
    while (first < last) {
        uint32_t mid = first + (last - first) / 2;
        if (SDL_memcmp(lane.entries[mid].hash.bytes, hash.bytes, sizeof(hash.bytes)) < 0)
            first = mid + 1;
        else
            last = mid;
    }
    return first;
}

bool create_artifact_cache(ArtifactCache& cache, ContentCache& content, ServerSignal& signal, uint32_t lanes, uint32_t buffers)
{
    if (!create_command_stream(cache.commands, signal, lanes, buffers))
        return false;
    cache.content = &content;
    cache.lanes = arena_allocate<ArtifactLane>(cache.commands.storage, lanes);
    cache.lane_count = lanes;
    for (uint32_t i = 0; i < lanes; ++i)
        cache.lanes[i] = {};
    for (uint32_t i = 0; i < lanes; ++i) {
        ArtifactLane& lane = cache.lanes[i];
        lane.lock = SDL_CreateRWLock();
        if (!lane.lock) {
            destroy_artifact_cache(cache);
            return false;
        }
        lane.capacity = 256;
        lane.entries = static_cast<ArtifactEntry*>(SDL_malloc(lane.capacity * sizeof(ArtifactEntry)));
    }
    return true;
}

void destroy_artifact_cache(ArtifactCache& cache)
{
    for (uint32_t i = 0; i < cache.lane_count; ++i) {
        for (uint32_t n = 0; n < cache.lanes[i].count; ++n)
            destroy_arena(cache.lanes[i].entries[n].storage);
        SDL_free(cache.lanes[i].entries);
        SDL_DestroyRWLock(cache.lanes[i].lock);
    }
    destroy_command_stream(cache.commands);
    cache = {};
}

uint64_t artifact_request(ArtifactCommandBuffer& buffer, const ArtifactKey& key)
{
    assert(key.kind != ArtifactKind::File);
    if (buffer.count == buffer.owner->command_limit)
        return 0;
    ArtifactCommand& command = command_append(buffer);
    command = {.request = buffer.owner->next_request++, .key = key, .hash = artifact_hash(key)};
    return command.request;
}

ArtifactView artifact_get(ArtifactCache& cache, const ContentHash& hash)
{
    ArtifactLane& lane = cache.lanes[artifact_owner(hash, cache.lane_count)];
    SDL_LockRWLockForReading(lane.lock);
    uint32_t pos = artifact_position(lane, hash);
    ArtifactView result = {};
    if (pos < lane.count && lane.entries[pos].hash == hash)
        result = lane.entries[pos].result;
    SDL_UnlockRWLock(lane.lock);
    return result;
}

static ArtifactView artifact_compute(ContentCache& content, const ArtifactKey& key, Arena& storage, char (&error)[160])
{
    if (key.kind == ArtifactKind::ImportGLTF && key.version == gltf_import_version) {
        if (key.parameters != ContentHash{})
            return {.state = ArtifactState::Failed, .error = ArtifactError::InvalidParameters};
        ContentView source = content_get(content, key.source);
        if (!source.data)
            return {.state = ArtifactState::Failed, .error = ArtifactError::MissingContent};
        ImportedScene* scene = arena_allocate<ImportedScene>(storage);
        *scene = {};
        if (!import_gltf(storage, {source.data, source.size}, *scene, error)) {
            destroy_arena(storage);
            return {.state = ArtifactState::Failed, .error = ArtifactError::InvalidData};
        }
        MeshAsset* meshes = arena_allocate<MeshAsset>(storage, scene->meshes.size);
        for (size_t i = 0; i < scene->meshes.size; ++i) {
            meshes[i] = {};
            if (!build_mesh_asset(storage, scene->meshes.data[i], meshes[i], error)) {
                destroy_arena(storage);
                return {.state = ArtifactState::Failed, .error = ArtifactError::InvalidData};
            }
        }
        return {.scene = scene, .meshes = {meshes, scene->meshes.size}, .state = ArtifactState::Ready};
    }
    if (key.kind != ArtifactKind::ByteHistogram || key.version != 1)
        return {.state = ArtifactState::Failed, .error = ArtifactError::Unsupported};
    ContentView source = content_get(content, key.source);
    if (!source.data)
        return {.state = ArtifactState::Failed, .error = ArtifactError::MissingContent};
    uint64_t offset = 0, size = source.size;
    if (key.parameters != ContentHash{}) {
        ContentView parameters = content_get(content, key.parameters);
        if (!parameters.data)
            return {.state = ArtifactState::Failed, .error = ArtifactError::MissingContent};
        if (parameters.size != 16)
            return {.state = ArtifactState::Failed, .error = ArtifactError::InvalidParameters};
        size = 0;
        for (uint32_t i = 0; i < 8; ++i) {
            offset |= uint64_t(parameters.data[i]) << (8 * i);
            size |= uint64_t(parameters.data[8 + i]) << (8 * i);
        }
        if (offset > source.size || size > source.size - offset)
            return {.state = ArtifactState::Failed, .error = ArtifactError::InvalidParameters};
    }
    ArenaTemp scratch = scratch_begin();
    uint64_t* counts = arena_allocate<uint64_t>(*scratch.arena, 256);
    SDL_memset(counts, 0, 256 * sizeof(uint64_t));
    for (size_t i = 0; i < size; ++i)
        ++counts[source.data[offset + i]];
    Arena bytes = {.block_capacity = 2048};
    uint8_t* output = arena_allocate<uint8_t>(bytes, 2048);
    for (uint32_t i = 0; i < 256; ++i) {
        for (uint32_t b = 0; b < 8; ++b)
            output[8 * i + b] = uint8_t(counts[i] >> (8 * b));
    }
    scratch_end(scratch);
    return {.content = content_publish(content, bytes, {output, 2048}), .state = ArtifactState::Ready};
}

static ArtifactEvent artifact_process(ArtifactCache& cache, ArtifactLane& lane, const ArtifactCommand& command)
{
    ArtifactEvent event = {.request = command.request, .artifact = command.hash, .kind = command.key.kind, .lane = get_thread_context()->thread_index};
    SDL_PathInfo info = {};
    ArtifactView probe = {};
    if (command.path)
        probe = file_probe(command.path, cache.file_size_limit, info, event.error);
    SDL_LockRWLockForWriting(lane.lock);
    uint32_t pos = artifact_position(lane, command.hash);
    if (pos == lane.count || lane.entries[pos].hash != command.hash) {
        if (lane.count == lane.capacity) {
            lane.capacity += 256;
            lane.entries = static_cast<ArtifactEntry*>(SDL_realloc(lane.entries, lane.capacity * sizeof(ArtifactEntry)));
        }
        SDL_memmove(lane.entries + pos + 1, lane.entries + pos, (lane.count - pos) * sizeof(ArtifactEntry));
        lane.entries[pos] = {.hash = command.hash};
        ++lane.count;
    }
    ArtifactEntry& entry = lane.entries[pos];
    uint64_t generation = 1;
    if (command.path) {
        bool changed = info.type != entry.file_info.type || info.size != entry.file_info.size || info.modify_time != entry.file_info.modify_time;
        generation = entry.result.generation + (changed || !entry.result.generation);
        entry.file_info = info;
    }
    if ((generation == entry.result.ready_generation && !command.path) ||
        (command.path && generation == entry.result.ready_generation && entry.result.state == ArtifactState::Ready && probe.state != ArtifactState::Failed)) {
        event.result = entry.result;
        event.cached = true;
        SDL_UnlockRWLock(lane.lock);
        return event;
    }
    entry.result.generation = generation;
    entry.result.state = ArtifactState::Building;
    entry.result.error = ArtifactError::None;
    SDL_UnlockRWLock(lane.lock);
    if (command.path) {
        event.result = probe;
        for (uint32_t attempt = 0; attempt < 3 && event.result.state != ArtifactState::Failed; ++attempt) {
            event.result = file_read(*cache.content, command.path, info, event.error);
            if (event.result.error != ArtifactError::Changed || attempt == 2)
                break;
            event.result = file_probe(command.path, cache.file_size_limit, info, event.error);
            SDL_LockRWLockForWriting(lane.lock);
            ++entry.result.generation;
            generation = entry.result.generation;
            entry.file_info = info;
            SDL_UnlockRWLock(lane.lock);
        }
    } else {
        event.result = artifact_compute(*cache.content, command.key, entry.storage, event.error);
    }
    SDL_LockRWLockForWriting(lane.lock);
    if (event.result.state == ArtifactState::Ready) {
        entry.result.content = event.result.content;
        entry.result.scene = event.result.scene;
        entry.result.meshes = event.result.meshes;
        entry.result.ready_generation = generation;
    }
    entry.result.state = event.result.state;
    entry.result.error = event.result.error;
    event.result = entry.result;
    SDL_UnlockRWLock(lane.lock);
    return event;
}

bool artifact_cache_tick(ArtifactCache& cache)
{
    ThreadContext& context = *get_thread_context();
    ArtifactCommandBuffer* buffer = nullptr;
    if (!ring_pop(cache.commands.lanes[context.thread_index], buffer))
        return false;
    ArtifactLane& lane = cache.lanes[context.thread_index];
    for (CommandChunk<ArtifactCommand, ArtifactEvent>* chunk = buffer->first; chunk; chunk = chunk->next) {
        for (uint32_t i = 0; i < chunk->count; ++i) {
            if (artifact_owner(chunk->commands[i].hash, context.thread_count) == context.thread_index)
                chunk->events[i] = artifact_process(cache, lane, chunk->commands[i]);
        }
    }
    commands_complete(*buffer);
    return true;
}
