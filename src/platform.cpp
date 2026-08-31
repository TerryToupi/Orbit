#include <SDL3/SDL.h>
#include <platform.hpp>

namespace Orbit::Platform
{

enum class PlatformResourceKind : u64
{
    NONE,
    MUTEX,
    RWLOCK,
    SEMAPHORE,
    CONDVAR,
    BARRIER,
    THREAD,
    WINDOW,
};

struct BarrierImpl
{
    SDL_Mutex     *lock;
    SDL_Condition *cv;
    u64 n, count, gen;
};

struct PlatformResource
{
    PlatformResourceKind kind;
    union
    {
        SDL_Mutex     *mutex;
        SDL_RWLock    *rwlock;
        SDL_Semaphore *semaphore;
        SDL_Condition *condvar;
        BarrierImpl    barrier;
        SDL_Thread    *thread;
        SDL_Window    *window;
    };
};
static Pool<PlatformResource> gResources;

template<typename T>
static constexpr u8 has_flag(T value, T flag)
{
    using U = std::underlying_type_t<T>;
    return (static_cast<U>(value) & static_cast<U>(flag)) != 0;
}

static inline SDL_WindowFlags toSDL(WindowFlags flags)
{
    SDL_WindowFlags result = 0;
    if (has_flag(flags, WindowFlags::RESIZABLE))
        result |= SDL_WINDOW_RESIZABLE;
    if (has_flag(flags, WindowFlags::BORDERLESS))
        result |= SDL_WINDOW_BORDERLESS;
    if (has_flag(flags, WindowFlags::FULLSCREEN))
        result |= SDL_WINDOW_FULLSCREEN;
    if (has_flag(flags, WindowFlags::INPUT_FOCUS))
        result |= SDL_WINDOW_INPUT_FOCUS;
    if (has_flag(flags, WindowFlags::MOUSE_FOCUS))
        result |= SDL_WINDOW_MOUSE_FOCUS;
    if (has_flag(flags, WindowFlags::UTILITY))
        result |= SDL_WINDOW_UTILITY;
    if (has_flag(flags, WindowFlags::TOOLTIP))
        result |= SDL_WINDOW_UTILITY;
    return result;
}

static inline SDL_ThreadPriority toSDL(ThreadPriority flag)
{
    switch (flag)
    {
        case ThreadPriority::LOW: return SDL_THREAD_PRIORITY_LOW;
        case ThreadPriority::NORMAL: return SDL_THREAD_PRIORITY_NORMAL;
        case ThreadPriority::HIGH: return SDL_THREAD_PRIORITY_HIGH;
        case ThreadPriority::CRITICAL: return SDL_THREAD_PRIORITY_TIME_CRITICAL;
    }
}

static inline ThreadState fromSDL(SDL_ThreadState flag)
{
    switch (flag)
    {
        case SDL_THREAD_UNKNOWN: return ThreadState::UNKNOWN;
        case SDL_THREAD_ALIVE: return ThreadState::ALIVE;
        case SDL_THREAD_DETACHED: return ThreadState::DETACHED;
        case SDL_THREAD_COMPLETE: return ThreadState::COMPLETE;
    }
}

void init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("Failed to initialized!");
        std::abort();
    }
}

void destroy()
{
    u32 resource_count = gResources.count();
    for (u32 r = 0; r < resource_count; ++r)
    {
        auto resource = gResources.at(r);
        if (resource)
        {
            switch (resource->kind) {
                case PlatformResourceKind::WINDOW:
                {
                    SDL_DestroyWindow(resource->window);
                    break;
                }
                case PlatformResourceKind::MUTEX:
                {
                    SDL_DestroyMutex(resource->mutex);
                    break;
                }
                case PlatformResourceKind::RWLOCK:
                {
                    SDL_DestroyRWLock(resource->rwlock);
                    break;
                }
                case PlatformResourceKind::SEMAPHORE:
                {
                    SDL_DestroySemaphore(resource->semaphore);
                    break;
                }
                case PlatformResourceKind::CONDVAR:
                {
                    SDL_DestroyCondition(resource->condvar);
                    break;
                }
                case PlatformResourceKind::BARRIER:
                {
                    SDL_DestroyCondition(resource->barrier.cv);
                    SDL_DestroyMutex(resource->barrier.lock);
                    break;
                }
                case PlatformResourceKind::THREAD:
                {
                    if (SDL_GetThreadState(resource->thread) != SDL_THREAD_COMPLETE)
                        SDL_Log("Thread <name>:%s <id>:%llu is still alive", SDL_GetThreadName(resource->thread), SDL_GetThreadID(resource->thread));
                    break;
                }
                case PlatformResourceKind::NONE:
                    break;
            }
        }
    }
    gResources.reset();
    
    SDL_Quit();
}

Handle<Window> create_window(WindowDesc&& desc)
{
    SDL_Window *window = SDL_CreateWindow(desc.name, (int)desc.width, (int)desc.height, toSDL(desc.props));
    auto h = gResources.emplace({.kind = PlatformResourceKind::WINDOW, .window = window});
    return Handle<Window>(h.idx, h.gen);
}

void destroy_window(Handle<Window> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::WINDOW);
    SDL_DestroyWindow(resource.window);
    gResources.erase({h.idx(), h.gen()});
}

Handle<Mutex> create_mutex()
{
    SDL_Mutex *mutex = SDL_CreateMutex();
    auto h = gResources.emplace({.kind = PlatformResourceKind::MUTEX, .mutex = mutex});
    return Handle<Mutex>(h.idx, h.gen);
}

void lock_mutex(Handle<Mutex> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::MUTEX);
    auto mutex = resource.mutex;
    
    SDL_LockMutex(mutex);
}

b32 try_lock_mutex(Handle<Mutex> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::MUTEX);
    auto mutex = resource.mutex;
    
    return (b32)SDL_TryLockMutex(mutex);
}

void unlock_mutex(Handle<Mutex> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::MUTEX);
    auto mutex = resource.mutex;
    
    SDL_UnlockMutex(mutex);
}

void destroy_mutex(Handle<Mutex> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::MUTEX);
    SDL_DestroyMutex(resource.mutex);
    gResources.erase({h.idx(), h.gen()});
}

Handle<RWlock> create_rwlock()
{
    SDL_RWLock *rwlock = SDL_CreateRWLock();
    auto h = gResources.emplace({.kind = PlatformResourceKind::RWLOCK, .rwlock = rwlock});
    return Handle<RWlock>(h.idx, h.gen);
}

void lock_read_rwlock(Handle<RWlock> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::RWLOCK);
    auto rwlock = resource.rwlock;
    
    SDL_LockRWLockForReading(rwlock);
}

void lock_write_rwlock(Handle<RWlock> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::RWLOCK);
    auto rwlock = resource.rwlock;
    
    SDL_LockRWLockForWriting(rwlock);
}

b32 try_lock_read_rwlock(Handle<RWlock> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::RWLOCK);
    auto rwlock = resource.rwlock;
    
    return (b32)SDL_TryLockRWLockForReading(rwlock);
}

b32 try_lock_write_rwlock(Handle<RWlock> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::RWLOCK);
    auto rwlock = resource.rwlock;
    
    return (b32)SDL_TryLockRWLockForWriting(rwlock);
}

void unlock_rwlock(Handle<RWlock> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::RWLOCK);
    auto rwlock = resource.rwlock;
    
    SDL_UnlockRWLock(rwlock);
}

void destroy_rwlock(Handle<RWlock> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::RWLOCK);
    SDL_DestroyRWLock(resource.rwlock);
    gResources.erase({h.idx(), h.gen()});
}

Handle<Semaphore> create_semaphore(u32 init_value)
{
    SDL_Semaphore *semaphore = SDL_CreateSemaphore(init_value);
    auto h = gResources.emplace({.kind = PlatformResourceKind::SEMAPHORE, .semaphore = semaphore});
    return Handle<Semaphore>(h.idx, h.gen);
}

void wait_semaphore(Handle<Semaphore> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::SEMAPHORE);
    auto semaphore = resource.semaphore;
    
    SDL_WaitSemaphore(semaphore);
}

b32 try_wait_semaphore(Handle<Semaphore> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::SEMAPHORE);
    auto semaphore = resource.semaphore;
        
    SDL_TryWaitSemaphore(semaphore);
}

b32 wait_semaphore_timeout(Handle<Semaphore> h, s32 ms)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::SEMAPHORE);
    auto semaphore = resource.semaphore;
        
    SDL_WaitSemaphoreTimeout(semaphore, (Sint32)ms);
}

void signal_semaphore(Handle<Semaphore> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::SEMAPHORE);
    auto semaphore = resource.semaphore;
        
    SDL_SignalSemaphore(semaphore);
}

u32 get_semaphore_value(Handle<Semaphore> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::SEMAPHORE);
    auto semaphore = resource.semaphore;
    
    return (u32)SDL_GetSemaphoreValue(semaphore);
}

void destroy_semaphore(Handle<Semaphore> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::SEMAPHORE);
    SDL_DestroySemaphore(resource.semaphore);
    gResources.erase({h.idx(), h.gen()});
}

Handle<CondVar> create_condition()
{
    SDL_Condition *condition = SDL_CreateCondition();
    auto h = gResources.emplace({.kind = PlatformResourceKind::CONDVAR, .condvar = condition});
    return Handle<CondVar>(h.idx, h.gen);
}

void signal_condition(Handle<CondVar> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::CONDVAR);
    auto cv = resource.condvar;
    
    SDL_SignalCondition(cv);
}

void broadcast_condition(Handle<CondVar> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::CONDVAR);
    auto cv = resource.condvar;
    
    SDL_BroadcastCondition(cv);
}

void wait_condition(Handle<CondVar> cv, Handle<Mutex> m)
{
    auto resource1 = gResources.at({cv.idx(), cv.gen()});
    assert(resource1.kind == PlatformResourceKind::CONDVAR);
    auto condvar = resource1.condvar;

    auto resource2 = gResources.at({m.idx(), m.gen()});
    assert(resource2.kind == PlatformResourceKind::MUTEX);
    auto mutex = resource2.mutex;
    
    SDL_WaitCondition(condvar, mutex);
}

b32 wait_condition_timeout(Handle<CondVar> cv, Handle<Mutex> m, s32 ms)
{
    auto resource1 = gResources.at({cv.idx(), cv.gen()});
    assert(resource1.kind == PlatformResourceKind::CONDVAR);
    auto condvar = resource1.condvar;

    auto resource2 = gResources.at({m.idx(), m.gen()});
    assert(resource2.kind == PlatformResourceKind::MUTEX);
    auto mutex = resource2.mutex;
    
    return (b32)SDL_WaitConditionTimeout(condvar, mutex, (Sint32)ms);
}

void destroy_condition(Handle<CondVar> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::CONDVAR);
    SDL_DestroyCondition(resource.condvar);
    gResources.erase({h.idx(), h.gen()});
}

Handle<Barrier> create_barrier(u32 n)
{
    auto h = gResources.emplace({.kind = PlatformResourceKind::BARRIER, .barrier = {
        .lock = SDL_CreateMutex(),
        .cv = SDL_CreateCondition(),
        .n = n,
        .count = 0,
        .gen = 0,
    }});
    return Handle<Barrier>(h.idx, h.gen);
}

void wait_barrier(Handle<Barrier> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::BARRIER);
    auto b = resource.barrier;
    
    SDL_LockMutex(b.lock);
    u64 my_gen = b.gen;

    if (++b.count == b.n)
    {
        b.count = 0;
        b.gen++;
        SDL_BroadcastCondition(b.cv);
    }
    else
    {
        while (my_gen == b.gen)
            SDL_WaitCondition(b.cv, b.lock);
    }
    
    SDL_UnlockMutex(b.lock);
}

void destroy_barrier(Handle<Barrier> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::BARRIER);
    auto b = resource.barrier;
    SDL_DestroyCondition(b.cv);
    SDL_DestroyMutex(b.lock);
    gResources.erase({h.idx(), h.gen()});
}

Handle<Thread> create_thread(ThreadDesc&& desc)
{
    SDL_ThreadFunction funct = reinterpret_cast<SDL_ThreadFunction>(desc.funct);
    SDL_Thread* thread = SDL_CreateThread(funct, desc.name, desc.data);
    auto h = gResources.emplace({.kind = PlatformResourceKind::THREAD, .thread = thread});
    SDL_Log("<ORBIT> Thread spawn NAME: %s, ID: %llu", SDL_GetThreadName(thread), SDL_GetThreadID(thread));
    return Handle<Thread>(h.idx, h.gen);
}

void wait_thread(Handle<Thread> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::THREAD);
    auto thread = resource.thread;
   
    int status = -1;
    const char *name = SDL_GetThreadName(thread);
    
    SDL_WaitThread(thread, (int *)&status);
    gResources.erase({h.idx(), h.gen()});

    if (status == 0)
        SDL_Log("<ORBIT> Thread NAME: %s exited gracefully!", name);
    else
        SDL_Log("<ORBIT> Thread NAME: %s exited with STATUS: %d!", name, status);
}

const char *get_thread_name(Handle<Thread> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::THREAD);
    auto thread = resource.thread;
    
    return SDL_GetThreadName(thread);
}

u64 get_thread_id(Handle<Thread> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::THREAD);
    auto thread = resource.thread;
    
    return (u64)SDL_GetThreadID(thread);
}

u64 get_current_thread_id(Handle<Thread> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::THREAD);
    auto thread = resource.thread;
    
    return (u64)SDL_GetCurrentThreadID();
}

void set_current_thread_priority(Handle<Thread> h, ThreadPriority p)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::THREAD);
    auto thread = resource.thread;
    
    SDL_SetCurrentThreadPriority(toSDL(p));
}

ThreadState get_thread_state(Handle<Thread> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::THREAD);
    auto thread = resource.thread;
    
    return fromSDL(SDL_GetThreadState(thread));
}

void detach_thread(Handle<Thread> h)
{
    auto resource = gResources.at({h.idx(), h.gen()});
    assert(resource.kind == PlatformResourceKind::THREAD);
    auto thread = resource.thread;
    
    SDL_DetachThread(thread);
}

}
