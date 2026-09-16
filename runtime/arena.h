#ifndef ORBIT_ARENA_H
#define ORBIT_ARENA_H

#include <cassert>
#include <cstddef>
#include <cstdint>

struct ArenaBlock;

// Owns its blocks. Do not copy an arena or move it while allocators or markers reference it.
struct Arena
{
    ArenaBlock* block = nullptr;
    size_t block_capacity = 64 * 1024;
};

struct ArenaTemp
{
    Arena* arena = nullptr;
    ArenaBlock* block = nullptr;
    size_t pos = 0;
};

// Returns uninitialized storage. Alignment must be a power of two; sizes including padding must fit size_t.
void* arena_allocate(Arena& arena, size_t size, size_t alignment = alignof(std::max_align_t));
// Reset retains the oldest block. Destroy releases all blocks. Neither runs object destructors.
void arena_reset(Arena& arena);
void destroy_arena(Arena& arena);

// End markers once, in reverse order within each arena, before resetting or destroying it.
ArenaTemp arena_temp_begin(Arena& arena);
void arena_temp_end(ArenaTemp temp);

template<typename T>
T* arena_allocate(Arena& arena, size_t count = 1)
{
    assert(count <= SIZE_MAX / sizeof(T));
    return static_cast<T*>(arena_allocate(arena, count * sizeof(T), alignof(T)));
}

// Containers must be destroyed before their storage is reclaimed. Copies and rebinds borrow the same arena.
template<typename T>
struct ArenaAllocator
{
    using value_type = T;

    Arena* arena;

    explicit ArenaAllocator(Arena& storage) noexcept : arena(&storage) {}

    template<typename U>
    ArenaAllocator(const ArenaAllocator<U>& other) noexcept : arena(other.arena) {}

    [[nodiscard]] T* allocate(size_t count)
    {
        return arena_allocate<T>(*arena, count);
    }

    void deallocate(T*, size_t) noexcept {}
};

template<typename T, typename U>
bool operator==(const ArenaAllocator<T>& left, const ArenaAllocator<U>& right) noexcept
{
    return left.arena == right.arena;
}

#endif
