#include <servers.h>
#include <thread_context.h>
#include <memory_setup.h>
#include <SDL3/SDL.h>
#include <list>
#include <vector>

#define CHECK(expression) do { if (!(expression)) { SDL_Log("%s:%d: %s (%s)", __FILE__, __LINE__, #expression, SDL_GetError()); return false; } } while (0)

struct alignas(256) AlignedValue
{
    uint32_t value = 0;
};

static bool test_arena_and_ring()
{
    Arena arena = {.block_capacity = 128};
    ArenaTemp empty = arena_temp_begin(arena);
    uint32_t* value = arena_allocate<uint32_t>(arena);
    *value = 51;
    ArenaTemp outer = arena_temp_begin(arena);
    void* position = arena_allocate(arena, 7, 1);
    ArenaTemp inner = arena_temp_begin(arena);
    AlignedValue* aligned = arena_allocate<AlignedValue>(arena, 100);
    CHECK(reinterpret_cast<uintptr_t>(aligned) % 256 == 0);
    arena_allocate(arena, 100000, 4096);
    CHECK(*value == 51);
    arena_temp_end(inner);
    arena_temp_end(outer);
    CHECK(arena_allocate(arena, 7, 1) == position);
    arena_temp_end(empty);
    CHECK(!arena.block);
    {
        std::vector<AlignedValue, ArenaAllocator<AlignedValue>> values{ArenaAllocator<AlignedValue>{arena}};
        for (uint32_t i = 0; i < 100; ++i)
            values.push_back({.value = i});
        CHECK(values[99].value == 99 && reinterpret_cast<uintptr_t>(values.data()) % 256 == 0);
        std::list<uint32_t, ArenaAllocator<uint32_t>> nodes{ArenaAllocator<uint32_t>{arena}};
        nodes.push_back(123);
        CHECK(nodes.front() == 123);
    }
    arena_reset(arena);
    Ring<uint32_t> ring;
    CHECK(create_ring(ring, arena, 3));
    for (uint32_t round = 0; round < 100; ++round) {
        for (uint32_t i = 0; i < 3; ++i)
            CHECK(ring_push(ring, round * 3 + i));
        CHECK(!ring_push(ring, 999u));
        for (uint32_t i = 0; i < 3; ++i) {
            uint32_t output = 0;
            CHECK(ring_pop(ring, output) && output == round * 3 + i);
        }
        uint32_t untouched = 123;
        CHECK(!ring_pop(ring, untouched) && untouched == 123);
    }
    SDL_DestroyMutex(ring.mutex);
    destroy_arena(arena);
    ArenaTemp scratch = scratch_begin();
    ArenaTemp other = scratch_begin({scratch.arena});
    CHECK(other.arena != scratch.arena);
    scratch_end(other);
    scratch_end(scratch);
    return true;
}

static bool hash_matches(Span<uint8_t> bytes, const char* hex)
{
    ContentHash key = content_hash(bytes);
    char output[65];
    for (uint32_t i = 0; i < 32; ++i)
        SDL_snprintf(output + i * 2, 3, "%02x", key.bytes[i]);
    CHECK(SDL_strcmp(output, hex) == 0);
    return true;
}

struct ContentReader
{
    ContentCache* cache = nullptr;
    ContentHash key = {};
    ContentView expected = {};
    SDL_Semaphore* start = nullptr;
    uint32_t index = 0;
};

static int SDLCALL content_reader(void* data)
{
    ContentReader& reader = *static_cast<ContentReader*>(data);
    ThreadContext context = {.thread_index = reader.index, .thread_count = 4};
    if (!set_thread_context(&context))
        return 1;
    SDL_WaitSemaphore(reader.start);
    int result = 0;
    for (uint32_t i = 0; i < 2000; ++i) {
        ContentView view = content_get(*reader.cache, reader.key);
        ContentHash duplicate = content_insert(*reader.cache, {reader.expected.data, reader.expected.size});
        if (view.data != reader.expected.data || view.size != reader.expected.size || duplicate != reader.key)
            result = 1;
        uint8_t shared[8] = {uint8_t(i), uint8_t(i >> 8), 42, 43, 44, 45, 46, 47};
        ContentHash inserted = content_insert(*reader.cache, shared);
        ContentView published = content_get(*reader.cache, inserted);
        if (published.size != sizeof(shared) || SDL_memcmp(published.data, shared, sizeof(shared)) != 0)
            result = 1;
    }
    destroy_thread_context(context);
    return result;
}

static bool test_content()
{
    CHECK(hash_matches({}, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    const uint8_t abc[] = {'a', 'b', 'c'};
    CHECK(hash_matches(abc, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"));
    const char* long_message = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    CHECK(hash_matches({reinterpret_cast<const uint8_t*>(long_message), SDL_strlen(long_message)},
                       "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"));
    ArenaTemp scratch = scratch_begin();
    uint8_t* million = arena_allocate<uint8_t>(*scratch.arena, 1000000);
    SDL_memset(million, 'a', 1000000);
    CHECK(hash_matches({million, 1000000}, "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"));
    scratch_end(scratch);
    ContentCache cache;
    CHECK(create_content_cache(cache));
    ContentHash empty = content_insert(cache, {});
    CHECK(content_get(cache, empty).data && content_get(cache, empty).size == 0);
    CHECK(!content_get(cache, {}).data);
    Arena storage = {.block_capacity = 3};
    uint8_t* original = arena_allocate<uint8_t>(storage, 3);
    SDL_memcpy(original, abc, 3);
    ContentHash key = content_publish(cache, storage, {original, 3});
    CHECK(!storage.block && content_get(cache, key).data == original);
    uint32_t entries = 0;
    for (const ContentStripe& stripe : cache.stripes)
        entries += stripe.count;
    CHECK(entries == 2);
    CHECK(content_insert(cache, abc) == key);
    uint8_t* duplicate = arena_allocate<uint8_t>(storage, 3);
    SDL_memcpy(duplicate, abc, 3);
    CHECK(content_publish(cache, storage, {duplicate, 3}) == key);
    CHECK(!storage.block && content_get(cache, key).data == original);
    SDL_Semaphore* gate = SDL_CreateSemaphore(0);
    CHECK(gate);
    ContentReader readers[4];
    SDL_Thread* threads[4];
    for (uint32_t i = 0; i < 4; ++i) {
        readers[i] = {.cache = &cache, .key = key, .expected = content_get(cache, key), .start = gate, .index = i};
        threads[i] = SDL_CreateThread(content_reader, "content-reader", &readers[i]);
        CHECK(threads[i]);
    }
    for (uint32_t i = 0; i < 4; ++i)
        SDL_SignalSemaphore(gate);
    for (uint32_t i = 0; i < 6000; ++i) {
        uint8_t bytes[4] = {uint8_t(i), uint8_t(i >> 8), uint8_t(i >> 16), uint8_t(i >> 24)};
        ContentHash inserted = content_insert(cache, bytes);
        ContentView view = content_get(cache, inserted);
        CHECK(view.size == 4 && SDL_memcmp(view.data, bytes, 4) == 0);
    }
    for (SDL_Thread* thread : threads) {
        int result = -1;
        SDL_WaitThread(thread, &result);
        CHECK(result == 0);
    }
    CHECK(content_get(cache, key).data == original && SDL_memcmp(original, abc, 3) == 0);
    entries = 0;
    for (const ContentStripe& stripe : cache.stripes)
        entries += stripe.count;
    CHECK(entries == 8002);
    SDL_DestroySemaphore(gate);
    destroy_content_cache(cache);
    return true;
}

static bool write_file(const char* path, Span<uint8_t> bytes)
{
    SDL_IOStream* io = SDL_IOFromFile(path, "wb");
    CHECK(io);
    size_t written = bytes.size ? SDL_WriteIO(io, bytes.data, bytes.size) : 0;
    bool closed = SDL_CloseIO(io);
    CHECK(written == bytes.size && closed);
    return true;
}

static bool wait_artifact(ArtifactCache& cache, ArtifactEvent& event)
{
    uint64_t deadline = SDL_GetTicks() + 10000;
    while (!command_next_event(cache.commands, event)) {
        CHECK(SDL_GetTicks() < deadline);
        SDL_Delay(1);
    }
    return true;
}

static uint64_t histogram_count(ContentView view, uint32_t value)
{
    uint64_t result = 0;
    for (uint32_t i = 0; i < 8; ++i)
        result |= uint64_t(view.data[value * 8 + i]) << (8 * i);
    return result;
}

static bool test_refresh(Servers& servers, const ArtifactKey& original)
{
    ContentHash slot = artifact_hash(original);
    ArtifactView old = artifact_get(servers.artifacts, slot);
    CHECK(old.state == ArtifactState::Ready && old.ready_generation == 1);
    const uint8_t replacement[] = {91, 92, 93, 91};
    ArtifactKey key = {.source = content_insert(servers.content, replacement)};
    ContentHash next = artifact_hash(key);
    SDL_RWLock* gate = servers.content.stripes[key.source.bytes[0] % 16].lock;
    SDL_LockRWLockForWriting(gate);
    for (uint32_t batch = 0; batch < 2; ++batch) {
        ArtifactCommandBuffer* commands = commands_begin(servers.artifacts.commands);
        CHECK(commands);
        for (uint32_t i = 0; i < 64; ++i)
            CHECK(artifact_request(*commands, key));
        commands_submit(*commands);
    }
    uint64_t deadline = SDL_GetTicks() + 10000;
    ArtifactView building;
    do {
        building = artifact_get(servers.artifacts, next);
        if (building.state == ArtifactState::Building)
            break;
        SDL_Delay(1);
    } while (SDL_GetTicks() < deadline);
    SDL_UnlockRWLock(gate);
    CHECK(building.state == ArtifactState::Building && !building.ready_generation);
    CHECK(artifact_get(servers.artifacts, slot).state == ArtifactState::Ready && artifact_get(servers.artifacts, slot).content == old.content);
    uint32_t builds = 0;
    ArtifactEvent event;
    for (uint32_t i = 0; i < 128; ++i) {
        CHECK(wait_artifact(servers.artifacts, event));
        CHECK(event.result.state == ArtifactState::Ready && event.result.content != old.content && event.artifact == next);
        builds += !event.cached;
    }
    CHECK(builds == 1);
    ContentHash ready = event.result.content;
    CHECK(histogram_count(content_get(servers.content, ready), 91) == 2);
    CHECK(histogram_count(content_get(servers.content, old.content), 'a') == 3);
    ArtifactCommandBuffer* commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    artifact_request(*commands, {});
    commands_submit(*commands);
    CHECK(wait_artifact(servers.artifacts, event));
    CHECK(event.result.state == ArtifactState::Failed && event.result.error == ArtifactError::MissingContent);
    CHECK(artifact_get(servers.artifacts, next).content == ready && artifact_get(servers.artifacts, slot).content == old.content);
    return true;
}

static bool test_artifacts(Servers& servers, const ContentHash& source)
{
    ArtifactKey key = {.source = source};
    ContentHash hash = artifact_hash(key);
    CHECK(artifact_get(servers.artifacts, hash).state == ArtifactState::Missing);
    for (uint32_t batch = 0; batch < 2; ++batch) {
        ArtifactCommandBuffer* commands = commands_begin(servers.artifacts.commands);
        CHECK(commands);
        for (uint32_t i = 0; i < 129; ++i)
            artifact_request(*commands, key);
        commands_submit(*commands);
    }
    uint32_t computed = 0;
    for (uint32_t i = 0; i < 258; ++i) {
        ArtifactEvent event;
        CHECK(wait_artifact(servers.artifacts, event));
        CHECK(event.artifact == hash && event.result.state == ArtifactState::Ready);
        computed += !event.cached;
        ContentView view = content_get(servers.content, event.result.content);
        CHECK(view.size == 2048 && histogram_count(view, 'a') == 3 && histogram_count(view, 'b') == 2 && histogram_count(view, 'c') == 1);
    }
    CHECK(computed == 1);
    CHECK(artifact_get(servers.artifacts, hash).state == ArtifactState::Ready);

    uint8_t parameters[16] = {1, 0, 0, 0, 0, 0, 0, 0, 3};
    ArtifactKey sliced = {.source = source, .parameters = content_insert(servers.content, parameters)};
    ArtifactCommandBuffer* commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    artifact_request(*commands, sliced);
    artifact_request(*commands, {.source = source, .version = 2});
    artifact_request(*commands, {.source = source, .kind = ArtifactKind(123)});
    artifact_request(*commands, {.source = source, .parameters = source});
    commands_submit(*commands);
    ArtifactEvent event;
    CHECK(wait_artifact(servers.artifacts, event));
    CHECK(event.result.state == ArtifactState::Ready && event.artifact != hash);
    ContentView view = content_get(servers.content, event.result.content);
    CHECK(view.size == 2048 && histogram_count(view, 'a') == 1 && histogram_count(view, 'b') == 1 && histogram_count(view, 'c') == 1);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.error == ArtifactError::Unsupported);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.error == ArtifactError::Unsupported);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.error == ArtifactError::InvalidParameters);

    const uint8_t later[] = {7, 8, 9};
    ArtifactKey retry = {.source = content_hash(later)};
    commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    artifact_request(*commands, retry);
    commands_submit(*commands);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.error == ArtifactError::MissingContent);
    CHECK(content_insert(servers.content, later) == retry.source);
    commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    artifact_request(*commands, retry);
    commands_submit(*commands);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.state == ArtifactState::Ready && !event.cached);
    commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    for (uint32_t i = 0; i < 1300; ++i)
        artifact_request(*commands, {.source = source, .version = i + 2});
    commands_submit(*commands);
    uint32_t lane_mask = 0;
    uint64_t previous = 0;
    for (uint32_t i = 0; i < 1300; ++i) {
        CHECK(wait_artifact(servers.artifacts, event) && event.result.error == ArtifactError::Unsupported);
        CHECK(artifact_get(servers.artifacts, event.artifact).state == ArtifactState::Failed);
        CHECK(!event.cached && (!previous || event.request == previous + 1));
        previous = event.request;
        lane_mask |= 1u << event.lane;
    }
    CHECK(lane_mask == (1u << servers.thread_count) - 1);
    CHECK(artifact_get(servers.artifacts, hash).state == ArtifactState::Ready);
    CHECK(test_refresh(servers, key));
    return true;
}

static bool test_servers(uint32_t lanes)
{
    Servers servers;
    CHECK(create_servers(servers, {.thread_count = lanes, .command_buffers = 3, .file_size_limit = 1024}));
    CHECK(get_thread_context()->thread_count == 1);
    char path[1024], empty_path[1024], missing[1024], large_path[1024];
    uint64_t stamp = SDL_GetTicksNS();
    SDL_snprintf(path, sizeof(path), "%s/orbit-%llu.bin", ORBIT_TEST_DIRECTORY, static_cast<unsigned long long>(stamp));
    SDL_snprintf(empty_path, sizeof(empty_path), "%s/orbit-empty-%llu.bin", ORBIT_TEST_DIRECTORY, static_cast<unsigned long long>(stamp));
    SDL_snprintf(missing, sizeof(missing), "%s/orbit-missing-%llu.bin", ORBIT_TEST_DIRECTORY, static_cast<unsigned long long>(stamp));
    SDL_snprintf(large_path, sizeof(large_path), "%s/orbit-large-%llu.bin", ORBIT_TEST_DIRECTORY, static_cast<unsigned long long>(stamp));
    const uint8_t bytes[] = {'a', 'b', 'c', 'a', 'b', 'a'};
    uint8_t large[1025] = {};
    CHECK(write_file(path, bytes) && write_file(empty_path, {}) && write_file(large_path, large));
    ArtifactCommandBuffer* commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    commands_discard(*commands);
    commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    char copied[1024];
    SDL_strlcpy(copied, path, sizeof(copied));
    uint64_t first = file_load(*commands, copied);
    copied[0] = '?';
    file_load(*commands, missing);
    file_load(*commands, empty_path);
    file_load(*commands, large_path);
    file_load(*commands, ORBIT_TEST_DIRECTORY);
    commands_submit(*commands);
    ArtifactEvent event;
    CHECK(wait_artifact(servers.artifacts, event) && event.request == first && event.result.state == ArtifactState::Ready);
    ContentHash source = event.result.content;
    ContentView view = content_get(servers.content, source);
    CHECK(event.result.generation == 1 && event.result.ready_generation == 1);
    CHECK(view.size == sizeof(bytes) && SDL_memcmp(view.data, bytes, sizeof(bytes)) == 0);
    CHECK(wait_artifact(servers.artifacts, event) && event.request == first + 1 && event.result.error == ArtifactError::IOError);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.state == ArtifactState::Ready);
    CHECK(content_get(servers.content, event.result.content).data && content_get(servers.content, event.result.content).size == 0);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.error == ArtifactError::TooLarge);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.error == ArtifactError::NotFile);
    CHECK(write_file(empty_path, bytes));
    commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    file_load(*commands, empty_path);
    commands_submit(*commands);
    CHECK(wait_artifact(servers.artifacts, event));
    CHECK(event.artifact == file_identity(empty_path) && event.artifact != file_identity(path));
    CHECK(event.result.content == source && event.result.generation == 2 && content_get(servers.content, event.result.content).data == view.data);
    CHECK(test_artifacts(servers, source));

    // Recycle the lane and completion rings repeatedly while workers can return to sleep between submissions.
    for (uint32_t i = 0; i < 12; ++i) {
        commands = commands_begin(servers.artifacts.commands);
        CHECK(commands);
        uint64_t id = file_load(*commands, path);
        commands_submit(*commands);
        CHECK(wait_artifact(servers.artifacts, event) && event.request == id && event.result.content == source);
    }
    const uint8_t changed[] = {42, 43};
    CHECK(write_file(path, changed));
    ContentHash replacement = content_hash(changed);
    SDL_RWLock* gate = servers.content.stripes[replacement.bytes[0] % 16].lock;
    SDL_LockRWLockForWriting(gate);
    commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    file_load(*commands, path);
    commands_submit(*commands);
    uint64_t deadline = SDL_GetTicks() + 10000;
    ArtifactView refreshing;
    do {
        refreshing = artifact_get(servers.artifacts, file_identity(path));
        if (refreshing.state == ArtifactState::Building)
            break;
        SDL_Delay(1);
    } while (SDL_GetTicks() < deadline);
    SDL_UnlockRWLock(gate);
    CHECK(refreshing.state == ArtifactState::Building && refreshing.generation == 2);
    CHECK(refreshing.ready_generation == 1 && refreshing.content == source);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.state == ArtifactState::Ready && event.result.content != source);
    CHECK(event.result.generation == 2 && event.result.ready_generation == 2);
    CHECK(SDL_memcmp(view.data, bytes, sizeof(bytes)) == 0);
    CHECK(SDL_RemovePath(path));
    commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    file_load(*commands, path);
    commands_submit(*commands);
    CHECK(wait_artifact(servers.artifacts, event) && event.result.error == ArtifactError::IOError);
    CHECK(event.result.generation == 3 && event.result.ready_generation == 2 && event.result.content == replacement);
    CHECK(write_file(path, changed));

    // Exercise the production pre/read/post validation window without a timing-dependent filesystem race.
    SDL_PathInfo before;
    char error[160] = {};
    CHECK(file_probe(empty_path, servers.artifacts.file_size_limit, before, error).state == ArtifactState::Building);
    const uint8_t unstable[] = {181, 182, 183};
    CHECK(write_file(empty_path, unstable));
    ArtifactView rejected = file_read(servers.content, empty_path, before, error);
    CHECK(rejected.state == ArtifactState::Failed && rejected.error == ArtifactError::Changed && rejected.content == ContentHash{});
    CHECK(!content_get(servers.content, content_hash(unstable)).data);

    for (uint32_t batch = 0; batch < 3; ++batch) {
        commands = commands_begin(servers.artifacts.commands);
        CHECK(commands);
        file_load(*commands, path);
        ArenaBlock* initial = commands->arena.block;
        for (uint32_t i = 1; i < 600; ++i)
            file_load(*commands, path);
        CHECK(commands->arena.block != initial);
        commands_submit(*commands);
    }
    CHECK(!commands_begin(servers.artifacts.commands));
    stop_servers(servers);
    uint32_t count = 0;
    while (command_next_event(servers.artifacts.commands, event)) {
        CHECK(event.result.state == ArtifactState::Ready && content_get(servers.content, event.result.content).size == sizeof(changed));
        ++count;
    }
    CHECK(count == 1800);
    destroy_servers(servers);
    CHECK(SDL_RemovePath(path) && SDL_RemovePath(empty_path) && SDL_RemovePath(large_path));
    return true;
}

static bool test_idle_shutdown()
{
    for (uint32_t i = 0; i < 8; ++i) {
        Servers servers;
        CHECK(create_servers(servers, {.thread_count = 3, .command_buffers = 1}));
        uint64_t deadline = SDL_GetTicks() + 10000;
        bool idle = false;
        uint64_t phase = 0;
        do {
            SDL_LockMutex(servers.signal.mutex);
            idle = servers.signal.idle && servers.signal.arrived == servers.thread_count;
            phase = servers.signal.phase;
            SDL_UnlockMutex(servers.signal.mutex);
            if (!idle)
                SDL_Delay(1);
        } while (!idle && SDL_GetTicks() < deadline);
        CHECK(idle);
        SDL_Delay(5);
        SDL_LockMutex(servers.signal.mutex);
        idle = servers.signal.idle && servers.signal.phase == phase;
        SDL_UnlockMutex(servers.signal.mutex);
        CHECK(idle);
        ArtifactCommandBuffer* discarded = commands_begin(servers.artifacts.commands);
        CHECK(discarded);
        file_load(*discarded, "unsubmitted");
        destroy_servers(servers);
    }
    return true;
}

static bool test_command_limit()
{
    Servers servers;
    CHECK(create_servers(servers, {.thread_count = 3, .command_buffers = 1, .commands_per_buffer = 65}));
    ArtifactCommandBuffer* commands = commands_begin(servers.artifacts.commands);
    CHECK(commands);
    for (uint32_t i = 0; i < 65; ++i)
        CHECK(artifact_request(*commands, {.version = 2}));
    ArenaBlock* block = commands->arena.block;
    CHECK(!artifact_request(*commands, {.version = 2}) && !file_load(*commands, "full"));
    CHECK(commands->count == 65 && commands->arena.block == block && !commands_begin(servers.artifacts.commands));
    commands_submit(*commands);
    stop_servers(servers);
    ArtifactEvent event;
    uint32_t count = 0;
    while (command_next_event(servers.artifacts.commands, event)) {
        CHECK(event.result.error == ArtifactError::Unsupported);
        ++count;
    }
    CHECK(count == 65);
    destroy_servers(servers);
    return true;
}

int main()
{
    if (!initialize_memory() || !SDL_Init(0)) {
        SDL_Log("Initialization failed: %s", SDL_GetError());
        return 1;
    }
    ThreadContext context;
    if (!set_thread_context(&context))
        return 1;
    if (!test_arena_and_ring() || !test_content() || !test_servers(1) || !test_servers(3) || !test_idle_shutdown() || !test_command_limit())
        return 1;
    destroy_thread_context(context);
    SDL_Quit();
    SDL_Log("Arena, content, file, artifact, queue and shutdown tests passed");
    return 0;
}
