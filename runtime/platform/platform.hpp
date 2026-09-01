#ifndef __ORBIT_PLATFORM__
#define __ORBIT_PLATFORM__

#include <core.hpp>

namespace Orbit::Platform
{

/* global platform init */
void init();

/* global platform destroy */
void destroy();

/* Window */
class Window;

enum class WindowFlags : u64
{
    RESIZABLE       = 1ull<<0,
    BORDERLESS      = 1ull<<1,
    FULLSCREEN      = 1ull<<2,
    INPUT_FOCUS     = 1ull<<3,
    MOUSE_FOCUS     = 1ull<<4,
    UTILITY         = 1ull<<5,
    TOOLTIP         = 1ull<<6,
    NONE            = ~0ull
};

constexpr WindowFlags operator|(WindowFlags lhs, WindowFlags rhs) noexcept
{
    return static_cast<WindowFlags>(static_cast<u64>(lhs) |
                                    static_cast<u64>(rhs));
}

constexpr WindowFlags operator&(WindowFlags lhs, WindowFlags rhs) noexcept
{
    return static_cast<WindowFlags>(static_cast<u64>(lhs) &
                                    static_cast<u64>(rhs));
}

struct WindowDesc
{
    const char *name = "Orbit";
    WindowFlags props = WindowFlags::RESIZABLE | WindowFlags::INPUT_FOCUS;
    u64 width = 1024;
    u64 height = 720;
};

Handle<Window> create_window(WindowDesc&& desc);
void           destroy_window(Handle<Window> h);

/* Mutex */
class Mutex;

Handle<Mutex> create_mutex();
void          lock_mutex(Handle<Mutex> h);
b32           try_lock_mutex(Handle<Mutex> h);
void          unlock_mutex(Handle<Mutex> h);
void          destroy_mutex(Handle<Mutex> h);

/* RWlock */
class RWlock;

Handle<RWlock> create_rwlock();
void           lock_read_rwlock(Handle<RWlock> h);
void           lock_write_rwlock(Handle<RWlock> h);
b32            try_lock_read_rwlock(Handle<RWlock> h);
b32            try_lock_write_rwlock(Handle<RWlock> h);
void           unlock_rwlock(Handle<RWlock> h);
void           destroy_rwlock(Handle<RWlock> h);

/* Semaphore */
class Semaphore;

Handle<Semaphore> create_semaphore(u32 init_value);
void              wait_semaphore(Handle<Semaphore> h);
b32               try_wait_semaphore(Handle<Semaphore> h);
b32               wait_semaphore_timeout(Handle<Semaphore> h, s32 ms);
void              signal_semaphore(Handle<Semaphore> h);
u32               get_semaphore_value(Handle<Semaphore> h);
void              destroy_semaphore(Handle<Semaphore> h);

/* conditional variable */
class CondVar;

Handle<CondVar> create_condition();
void            signal_condition(Handle<CondVar> h);
void            broadcast_condition(Handle<CondVar> h);
void            wait_condition(Handle<CondVar> cv, Handle<Mutex> m);
b32             wait_condition_timeout(Handle<CondVar> cv, Handle<Mutex> m, s32 ms);
void            destroy_condition(Handle<CondVar> h);

/* Barrier */
class Barrier;

Handle<Barrier> create_barrier(u32 n);
void            wait_barrier(Handle<Barrier> h);
void            destroy_barrier(Handle<Barrier> h);

/* Thread */
class Thread;

typedef int(*ThreadFn)(void *);

struct ThreadDesc
{
    const char *name = "Orbit Thread";
    ThreadFn    funct = nullptr;
    void       *data = nullptr;
};

enum class ThreadPriority : u64
{
    LOW,
    NORMAL,
    HIGH,
    CRITICAL,
};

enum class ThreadState : u64
{
    UNKNOWN,
    ALIVE,
    DETACHED,
    COMPLETE,
};

Handle<Thread>  create_thread(ThreadDesc&& desc);
void            wait_thread(Handle<Thread> h);
const char     *get_thread_name(Handle<Thread> h);
u64             get_thread_id(Handle<Thread> h);
u64             get_current_thread_id(Handle<Thread> h);
void            set_current_thread_priority(Handle<Thread> h, ThreadPriority p);
ThreadState     get_thread_state(Handle<Thread> h);
void            detach_thread(Handle<Thread> h);

};

#endif
