#ifndef ORBIT_THREAD_CONTEXT_H
#define ORBIT_THREAD_CONTEXT_H

#include <arena.h>
#include <span.h>

// One owner thread per context. Do not copy contexts that own scratch blocks.
struct ThreadContext
{
    uint32_t thread_index = 0;
    uint32_t thread_count = 1;
    Arena scratch[2] = {};
};

// After SDL initialization, borrows context in SDL TLS and prepares its scratch blocks. Null clears the binding.
// Check failure during thread initialization; SDL_GetError describes TLS setup failures.
bool set_thread_context(ThreadContext* context);
ThreadContext* get_thread_context();
// Call on the owner thread after all scratch scopes end; clears its binding if current.
void destroy_thread_context(ThreadContext& context);

// Requires a current context and at least one nonconflicting arena. Pass arenas whose allocations must survive this scope.
// Nested scopes on the same arena must end in reverse order.
ArenaTemp scratch_begin(Span<Arena*> conflicts = {});
void scratch_end(ArenaTemp scratch);

#endif
