#include <file_stream.h>
#include <artifact_cache.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_error.h>

ContentHash file_identity(const char* path)
{
    assert(path);
    return artifact_hash({.source = content_hash({reinterpret_cast<const uint8_t*>(path), SDL_strlen(path)}), .kind = ArtifactKind::File});
}

uint64_t file_load(ArtifactCommandBuffer& buffer, const char* path)
{
    assert(path && buffer.state == CommandBufferState::Recording);
    if (buffer.count == buffer.owner->command_limit)
        return 0;
    size_t size = SDL_strlen(path) + 1;
    char* copied = arena_allocate<char>(buffer.arena, size);
    SDL_memcpy(copied, path, size);
    ArtifactCommand& command = command_append(buffer);
    command = {.request = buffer.owner->next_request++, .key = {.kind = ArtifactKind::File}, .hash = file_identity(path), .path = copied};
    return command.request;
}

ArtifactView file_probe(const char* path, uint64_t size_limit, SDL_PathInfo& info, char (&error)[160])
{
    info = {};
    if (!SDL_GetPathInfo(path, &info)) {
        SDL_strlcpy(error, SDL_GetError(), sizeof(error));
        return {.state = ArtifactState::Failed, .error = ArtifactError::IOError};
    }
    if (info.type != SDL_PATHTYPE_FILE)
        return {.state = ArtifactState::Failed, .error = ArtifactError::NotFile};
    if (info.size > size_limit)
        return {.state = ArtifactState::Failed, .error = ArtifactError::TooLarge};
    return {.state = ArtifactState::Building};
}

ArtifactView file_read(ContentCache& content, const char* path, const SDL_PathInfo& before, char (&error)[160])
{
    SDL_IOStream* io = SDL_IOFromFile(path, "rb");
    if (!io) {
        SDL_strlcpy(error, SDL_GetError(), sizeof(error));
        return {.state = ArtifactState::Failed, .error = ArtifactError::IOError};
    }
    Arena storage = {.block_capacity = size_t(before.size)};
    uint8_t* data = arena_allocate<uint8_t>(storage, size_t(before.size));
    size_t read = 0;
    while (read < before.size) {
        size_t count = SDL_ReadIO(io, data + read, size_t(before.size) - read);
        if (!count) {
            SDL_strlcpy(error, SDL_GetError(), sizeof(error));
            break;
        }
        read += count;
    }
    bool closed = SDL_CloseIO(io);
    if (!closed)
        SDL_strlcpy(error, SDL_GetError(), sizeof(error));
    SDL_PathInfo after;
    bool measured = SDL_GetPathInfo(path, &after);
    if (!measured)
        SDL_strlcpy(error, SDL_GetError(), sizeof(error));
    if (!measured || before.size != after.size || before.modify_time != after.modify_time || after.type != SDL_PATHTYPE_FILE) {
        destroy_arena(storage);
        return {.state = ArtifactState::Failed, .error = ArtifactError::Changed};
    }
    if (read != before.size || !closed) {
        destroy_arena(storage);
        return {.state = ArtifactState::Failed, .error = ArtifactError::IOError};
    }
    error[0] = 0;
    return {.content = content_publish(content, storage, {data, read}), .state = ArtifactState::Ready};
}
