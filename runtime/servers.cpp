#include <servers.h>
#include <thread_context.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_stdinc.h>

static int SDLCALL server_thread_entry(void* data)
{
    ServerWorker& worker = *static_cast<ServerWorker*>(data);
    Servers& servers = *worker.servers;
    ThreadContext context = {.thread_index = worker.index, .thread_count = servers.thread_count};
    bool initialized = set_thread_context(&context);
    SDL_LockMutex(servers.signal.mutex);
    if (!initialized) {
        servers.startup_failed = true;
        if (!servers.startup_error[0])
            SDL_strlcpy(servers.startup_error, SDL_GetError(), sizeof(servers.startup_error));
    }
    ++servers.initialized_threads;
    SDL_BroadcastCondition(servers.signal.changed);
    SDL_UnlockMutex(servers.signal.mutex);
    if (!initialized)
        return 1;

    SDL_LockMutex(servers.signal.mutex);
    while (!servers.signal.started && !servers.signal.stopping)
        SDL_WaitCondition(servers.signal.changed, servers.signal.mutex);
    bool started = servers.signal.started;
    SDL_UnlockMutex(servers.signal.mutex);
    if (!started) {
        destroy_thread_context(context);
        return 0;
    }
    for (;;) {
        SDL_LockMutex(servers.signal.mutex);
        uint64_t observed = servers.signal.generation;
        SDL_UnlockMutex(servers.signal.mutex);
        while (artifact_cache_tick(servers.artifacts)) {}

        // All lanes leave work before the group can sleep or stop. No lane sleeps inside a producer algorithm.
        SDL_LockMutex(servers.signal.mutex);
        uint64_t phase = servers.signal.phase;
        if (!servers.signal.arrived || observed < servers.signal.drained_generation)
            servers.signal.drained_generation = observed;
        if (++servers.signal.arrived == servers.thread_count) {
            servers.signal.idle = true;
            while (!servers.signal.stopping && servers.signal.drained_generation == servers.signal.generation)
                SDL_WaitCondition(servers.signal.changed, servers.signal.mutex);
            servers.signal.idle = false;
            servers.signal.finished = servers.signal.stopping && servers.signal.drained_generation == servers.signal.generation;
            servers.signal.arrived = 0;
            ++servers.signal.phase;
            SDL_BroadcastCondition(servers.signal.changed);
        } else {
            while (phase == servers.signal.phase)
                SDL_WaitCondition(servers.signal.changed, servers.signal.mutex);
        }
        bool stopping = servers.signal.finished;
        SDL_UnlockMutex(servers.signal.mutex);
        if (stopping)
            break;
    }
    destroy_thread_context(context);
    return 0;
}

bool create_servers(Servers& servers, const ServerDesc& desc)
{
    assert(desc.thread_count && desc.command_buffers && desc.commands_per_buffer);
    servers.signal.mutex = SDL_CreateMutex();
    if (!servers.signal.mutex)
        return false;
    servers.signal.changed = SDL_CreateCondition();
    if (!servers.signal.changed) {
        destroy_servers(servers);
        return false;
    }
    if (!create_content_cache(servers.content)) {
        destroy_servers(servers);
        return false;
    }
    servers.thread_count = desc.thread_count;
    if (!create_artifact_cache(servers.artifacts, servers.content, servers.signal, desc.thread_count, desc.command_buffers)) {
        destroy_servers(servers);
        return false;
    }
    servers.artifacts.file_size_limit = desc.file_size_limit;
    servers.artifacts.commands.command_limit = desc.commands_per_buffer;
    servers.workers = arena_allocate<ServerWorker>(servers.storage, desc.thread_count);
    for (uint32_t i = 0; i < desc.thread_count; ++i) {
        servers.workers[i] = {.servers = &servers, .index = i};
        char name[32];
        SDL_snprintf(name, sizeof(name), "server-%u", i);
        servers.workers[i].thread = SDL_CreateThread(server_thread_entry, name, &servers.workers[i]);
        if (!servers.workers[i].thread) {
            destroy_servers(servers);
            return false;
        }
        ++servers.created_threads;
    }
    SDL_LockMutex(servers.signal.mutex);
    while (servers.initialized_threads != desc.thread_count)
        SDL_WaitCondition(servers.signal.changed, servers.signal.mutex);
    bool failed = servers.startup_failed;
    SDL_UnlockMutex(servers.signal.mutex);
    if (failed) {
        char error[256];
        SDL_strlcpy(error, servers.startup_error, sizeof(error));
        destroy_servers(servers);
        return SDL_SetError("Server thread initialization: %s", error);
    }
    SDL_LockMutex(servers.signal.mutex);
    servers.signal.accepting = true;
    servers.signal.started = true;
    SDL_BroadcastCondition(servers.signal.changed);
    SDL_UnlockMutex(servers.signal.mutex);
    return true;
}

void stop_servers(Servers& servers)
{
    servers.signal.accepting = false;
    if (!servers.created_threads)
        return;
    SDL_LockMutex(servers.signal.mutex);
    servers.signal.stopping = true;
    SDL_BroadcastCondition(servers.signal.changed);
    SDL_UnlockMutex(servers.signal.mutex);
    for (uint32_t i = 0; i < servers.created_threads; ++i) {
        SDL_WaitThread(servers.workers[i].thread, nullptr);
        servers.workers[i].thread = nullptr;
    }
    servers.created_threads = 0;
}

void destroy_servers(Servers& servers)
{
    stop_servers(servers);
    destroy_artifact_cache(servers.artifacts);
    destroy_content_cache(servers.content);
    SDL_DestroyCondition(servers.signal.changed);
    SDL_DestroyMutex(servers.signal.mutex);
    destroy_arena(servers.storage);
    servers = {};
}
