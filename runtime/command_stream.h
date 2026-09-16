#ifndef ORBIT_COMMAND_STREAM_H
#define ORBIT_COMMAND_STREAM_H

#include <arena.h>
#include <SDL3/SDL_atomic.h>
#include <SDL3/SDL_mutex.h>

struct ServerSignal
{
    SDL_Mutex* mutex = nullptr;
    SDL_Condition* changed = nullptr;
    uint64_t generation = 0;
    uint64_t phase = 0;
    uint64_t drained_generation = 0;
    uint32_t arrived = 0;
    bool started = false;
    bool idle = false;
    bool finished = false;
    bool stopping = false;
    bool accepting = false; // main thread only
};

// Bounded rings transfer values under a short lock. No allocation or waiting for queue space.
template<typename T>
struct Ring
{
    SDL_Mutex* mutex = nullptr;
    T* data = nullptr;
    uint32_t capacity = 0;
    uint32_t read = 0;
    uint32_t count = 0;
};

template<typename T>
bool create_ring(Ring<T>& ring, Arena& storage, uint32_t capacity)
{
    assert(capacity);
    ring.mutex = SDL_CreateMutex();
    if (!ring.mutex)
        return false;
    ring.data = arena_allocate<T>(storage, capacity);
    ring.capacity = capacity;
    return true;
}

template<typename T>
bool ring_push(Ring<T>& ring, T value)
{
    SDL_LockMutex(ring.mutex);
    bool available = ring.count < ring.capacity;
    if (available) {
        ring.data[(ring.read + ring.count) % ring.capacity] = value;
        ++ring.count;
    }
    SDL_UnlockMutex(ring.mutex);
    return available;
}

template<typename T>
bool ring_pop(Ring<T>& ring, T& value)
{
    SDL_LockMutex(ring.mutex);
    bool available = ring.count != 0;
    if (available) {
        value = ring.data[ring.read];
        ring.read = (ring.read + 1) % ring.capacity;
        --ring.count;
    }
    SDL_UnlockMutex(ring.mutex);
    return available;
}

enum class CommandBufferState : uint32_t { Free, Recording, Submitted };

template<typename Command, typename Event> struct CommandStream;

template<typename Command, typename Event>
struct CommandChunk
{
    CommandChunk* next = nullptr;
    uint32_t count = 0;
    Command commands[64];
    Event events[64];
};

template<typename Command, typename Event>
struct CommandBuffer
{
    CommandStream<Command, Event>* owner = nullptr;
    Arena arena = {};
    CommandChunk<Command, Event>* first = nullptr;
    CommandChunk<Command, Event>* last = nullptr;
    SDL_AtomicInt remaining = {};
    CommandBufferState state = CommandBufferState::Free;
    uint32_t count = 0;
};

// One main producer broadcasts immutable buffers to SPSC lane rings. The last completing lane uses the MPSC completed ring.
// Completion storage is reserved by the buffer pool: at most buffer_count buffers can be outstanding, including unread results.
template<typename Command, typename Event>
struct CommandStream
{
    ServerSignal* signal = nullptr;
    Arena storage = {};
    CommandBuffer<Command, Event>* buffers = nullptr;
    Ring<CommandBuffer<Command, Event>*>* lanes = nullptr;
    Ring<CommandBuffer<Command, Event>*> completed = {};
    CommandBuffer<Command, Event>* reading = nullptr;
    CommandChunk<Command, Event>* read_chunk = nullptr;
    uint32_t read_index = 0;
    uint32_t buffer_count = 0;
    uint32_t lane_count = 0;
    uint32_t command_limit = 4096;
    uint64_t next_request = 1;
};

template<typename Command, typename Event>
void destroy_command_stream(CommandStream<Command, Event>& stream)
{
    for (uint32_t i = 0; i < stream.buffer_count; ++i)
        destroy_arena(stream.buffers[i].arena);
    for (uint32_t i = 0; i < stream.lane_count; ++i)
        SDL_DestroyMutex(stream.lanes[i].mutex);
    SDL_DestroyMutex(stream.completed.mutex);
    destroy_arena(stream.storage);
    stream = {};
}

template<typename Command, typename Event>
bool create_command_stream(CommandStream<Command, Event>& stream, ServerSignal& signal, uint32_t lanes, uint32_t buffers)
{
    assert(lanes && buffers);
    stream.signal = &signal;
    stream.buffers = arena_allocate<CommandBuffer<Command, Event>>(stream.storage, buffers);
    stream.lanes = arena_allocate<Ring<CommandBuffer<Command, Event>*>>(stream.storage, lanes);
    stream.buffer_count = buffers;
    stream.lane_count = lanes;
    for (uint32_t i = 0; i < buffers; ++i)
        stream.buffers[i] = {.owner = &stream};
    for (uint32_t i = 0; i < lanes; ++i)
        stream.lanes[i] = {};
    if (!create_ring(stream.completed, stream.storage, buffers)) {
        destroy_command_stream(stream);
        return false;
    }
    for (uint32_t i = 0; i < lanes; ++i) {
        if (!create_ring(stream.lanes[i], stream.storage, buffers)) {
            destroy_command_stream(stream);
            return false;
        }
    }
    return true;
}

// Main-thread APIs. Null begin means backpressure: consume results or discard an unsubmitted buffer, then retry.
template<typename Command, typename Event>
CommandBuffer<Command, Event>* commands_begin(CommandStream<Command, Event>& stream)
{
    assert(stream.signal->accepting);
    for (uint32_t i = 0; i < stream.buffer_count; ++i) {
        if (stream.buffers[i].state == CommandBufferState::Free) {
            stream.buffers[i].state = CommandBufferState::Recording;
            return &stream.buffers[i];
        }
    }
    return nullptr;
}

template<typename Command, typename Event>
Command& command_append(CommandBuffer<Command, Event>& buffer)
{
    assert(buffer.state == CommandBufferState::Recording && buffer.count < buffer.owner->command_limit);
    if (!buffer.last || buffer.last->count == 64) {
        CommandChunk<Command, Event>* chunk = arena_allocate<CommandChunk<Command, Event>>(buffer.arena);
        chunk->next = nullptr;
        chunk->count = 0;
        if (buffer.last)
            buffer.last->next = chunk;
        else
            buffer.first = chunk;
        buffer.last = chunk;
    }
    ++buffer.count;
    return buffer.last->commands[buffer.last->count++];
}

template<typename Command, typename Event>
void commands_discard(CommandBuffer<Command, Event>& buffer)
{
    assert(buffer.state == CommandBufferState::Recording);
    arena_reset(buffer.arena);
    buffer.first = buffer.last = nullptr;
    buffer.count = 0;
    buffer.state = CommandBufferState::Free;
}

template<typename Command, typename Event>
void commands_submit(CommandBuffer<Command, Event>& buffer)
{
    CommandStream<Command, Event>& stream = *buffer.owner;
    assert(stream.signal->accepting && buffer.state == CommandBufferState::Recording && buffer.count);
    buffer.state = CommandBufferState::Submitted;
    SDL_SetAtomicInt(&buffer.remaining, int(stream.lane_count));
    for (uint32_t i = 0; i < stream.lane_count; ++i) {
        bool pushed = ring_push(stream.lanes[i], &buffer);
        assert(pushed);
        (void)pushed;
    }
    SDL_LockMutex(stream.signal->mutex);
    ++stream.signal->generation;
    SDL_BroadcastCondition(stream.signal->changed);
    SDL_UnlockMutex(stream.signal->mutex);
}

// Each lane calls once after writing its disjoint event slots. SDL atomics publish writes to the last lane.
template<typename Command, typename Event>
void commands_complete(CommandBuffer<Command, Event>& buffer)
{
    if (SDL_AtomicDecRef(&buffer.remaining)) {
        bool pushed = ring_push(buffer.owner->completed, &buffer);
        assert(pushed);
        (void)pushed;
    }
}

// Copies one event to the main thread. Consuming the final event recycles its buffer and all recording memory.
template<typename Command, typename Event>
bool command_next_event(CommandStream<Command, Event>& stream, Event& event)
{
    if (!stream.reading) {
        if (!ring_pop(stream.completed, stream.reading))
            return false;
        stream.read_chunk = stream.reading->first;
        stream.read_index = 0;
    }
    event = stream.read_chunk->events[stream.read_index++];
    if (stream.read_index == stream.read_chunk->count) {
        stream.read_chunk = stream.read_chunk->next;
        stream.read_index = 0;
        if (!stream.read_chunk) {
            stream.reading->state = CommandBufferState::Recording;
            commands_discard(*stream.reading);
            stream.reading = nullptr;
        }
    }
    return true;
}

#endif
