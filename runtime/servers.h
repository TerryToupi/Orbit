#ifndef ORBIT_SERVERS_H
#define ORBIT_SERVERS_H

#include <file_stream.h>
#include <artifact_cache.h>
#include <SDL3/SDL_thread.h>

struct Servers;

struct ServerWorker
{
    Servers* servers = nullptr;
    SDL_Thread* thread = nullptr;
    uint32_t index = 0;
};

struct ServerDesc
{
    uint32_t thread_count = 2;
    uint32_t command_buffers = 32; // shared pool, including unread completion buffers
    uint32_t commands_per_buffer = 4096;
    uint64_t file_size_limit = 1024ull * 1024 * 1024;
};

// Main-thread owned, noncopyable by contract. Worker-visible configuration is immutable between create and stop.
struct Servers
{
    ContentCache content = {};
    ArtifactCache artifacts = {};
    ServerSignal signal = {};
    Arena storage = {};
    ServerWorker* workers = nullptr;
    uint32_t thread_count = 0;
    uint32_t created_threads = 0;
    uint32_t initialized_threads = 0;
    bool startup_failed = false;
    char startup_error[256] = {};
};

// SDL and its memory hooks must already be initialized. Checks all thread/TLS/synchronization initialization.
bool create_servers(Servers& servers, const ServerDesc& desc = {});
// Stops, discards remaining event/recording buffers, then destroys caches. No borrowed content views may remain.
void destroy_servers(Servers& servers);
// Stops accepting commands and drains submitted work before joining. Events and content remain available afterward.
// Does not wait for the main thread to consume events. Blocking filesystem calls must return before stop can complete.
void stop_servers(Servers& servers);

#endif
