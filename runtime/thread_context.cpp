#include <thread_context.h>

#include <SDL3/SDL_thread.h>

static SDL_TLSID thread_context_id;

bool set_thread_context(ThreadContext* context)
{
    if (!SDL_SetTLS(&thread_context_id, context, nullptr))
        return false;
    if (context) {
        for (Arena& arena : context->scratch) {
            if (!arena.block)
                arena_allocate(arena, 0);
        }
    }
    return true;
}

ThreadContext* get_thread_context()
{
    return static_cast<ThreadContext*>(SDL_GetTLS(&thread_context_id));
}

void destroy_thread_context(ThreadContext& context)
{
    if (get_thread_context() == &context)
        SDL_SetTLS(&thread_context_id, nullptr, nullptr);
    for (Arena& arena : context.scratch)
        destroy_arena(arena);
}

ArenaTemp scratch_begin(Span<Arena*> conflicts)
{
    ThreadContext* context = get_thread_context();
    assert(context);
    assert(conflicts.data || conflicts.size == 0);
    for (Arena& arena : context->scratch) {
        bool conflict = false;
        for (size_t i = 0; i < conflicts.size; ++i) {
            if (&arena == conflicts.data[i]) {
                conflict = true;
                break;
            }
        }
        if (!conflict)
            return arena_temp_begin(arena);
    }
    assert(false && "All scratch arenas conflict");
    return {};
}

void scratch_end(ArenaTemp scratch)
{
    arena_temp_end(scratch);
}
