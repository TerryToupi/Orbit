#include <arena.h>

#include <SDL3/SDL_stdinc.h>

struct ArenaBlock
{
    ArenaBlock* prev;
    size_t pos;
    size_t capacity;
};

void* arena_allocate(Arena& arena, size_t size, size_t alignment)
{
    assert(alignment && (alignment & (alignment - 1)) == 0);
    ArenaBlock* block = arena.block;
    if (block) {
        uintptr_t address = reinterpret_cast<uintptr_t>(block + 1) + block->pos;
        size_t padding = (0 - address) & (alignment - 1);
        if (padding <= block->capacity - block->pos && size <= block->capacity - block->pos - padding) {
            block->pos += padding + size;
            return reinterpret_cast<void*>(address + padding);
        }
    }

    assert(size <= SIZE_MAX - sizeof(ArenaBlock) - (alignment - 1));
    assert(arena.block_capacity <= SIZE_MAX - sizeof(ArenaBlock) - (alignment - 1));
    size_t capacity = (size > arena.block_capacity ? size : arena.block_capacity) + alignment - 1;
    block = static_cast<ArenaBlock*>(SDL_malloc(sizeof(ArenaBlock) + capacity));
    uintptr_t address = reinterpret_cast<uintptr_t>(block + 1);
    size_t padding = (0 - address) & (alignment - 1);
    *block = {.prev = arena.block, .pos = padding + size, .capacity = capacity};
    arena.block = block;
    return reinterpret_cast<void*>(address + padding);
}

void arena_reset(Arena& arena)
{
    while (arena.block && arena.block->prev) {
        ArenaBlock* block = arena.block;
        arena.block = block->prev;
        SDL_free(block);
    }
    if (arena.block)
        arena.block->pos = 0;
}

void destroy_arena(Arena& arena)
{
    while (arena.block) {
        ArenaBlock* block = arena.block;
        arena.block = block->prev;
        SDL_free(block);
    }
}

ArenaTemp arena_temp_begin(Arena& arena)
{
    return {.arena = &arena, .block = arena.block, .pos = arena.block ? arena.block->pos : 0};
}

void arena_temp_end(ArenaTemp temp)
{
    assert(temp.arena);
    while (temp.arena->block != temp.block) {
        ArenaBlock* block = temp.arena->block;
        assert(block);
        temp.arena->block = block->prev;
        SDL_free(block);
    }
    if (temp.block) {
        assert(temp.pos <= temp.block->pos);
        temp.block->pos = temp.pos;
    }
}
