#ifndef __ORBIT_PLATFORM__
#define __ORBIT_PLATFORM__

#include <core.hpp>

namespace Orbit::Platform
{

void init();
void destroy();

class Mutex;
Handle<Mutex> create_mutex();
void          lock_mutex(Handle<Mutex> h);
b32           try_lock_mutex(Handle<Mutex> h);
void          unlock_mutex(Handle<Mutex> h);
void          destroy_mutex(Handle<Mutex> h);

class RWlock;
Handle<RWlock> create_rwlock();
void           lock_read_rwlock(Handle<RWlock> h);
void           lock_write_rwlock(Handle<RWlock> h);
b32            try_lock_read_rwlock(Handle<RWlock> h);
b32            try_lock_write_rwlock(Handle<RWlock> h);
void           unlock_rwlock(Handle<RWlock> h);
void           destroy_rwlock(Handle<RWlock> h);

class Semaphore;
Handle<Semaphore> create_semaphore(u32 init_value);
void              wait_semaphore(Handle<Semaphore> h);
b32               try_wait_semaphore(Handle<Semaphore> h);
b32               wait_semaphore_timeout(Handle<Semaphore> h, s32 ms);
void              signal_semaphore(Handle<Semaphore> h);
u32               get_semaphore_value(Handle<Semaphore> h);
void              destroy_semaphore(Handle<Semaphore> h);

class CondVar;
Handle<CondVar> create_condition();
void            signal_condition(Handle<CondVar> h);
void            broadcast_condition(Handle<CondVar> h);
void            wait_condition(Handle<CondVar> cv, Handle<Mutex> m);
b32             wait_condition_timeout(Handle<CondVar> cv, Handle<Mutex> m, s32 ms);
void            destroy_condition(Handle<CondVar> h);

class Barrier;
Handle<Barrier> create_barrier(u32 n);
void            wait_barrier(Handle<Barrier> h);
void            destroy_barrier(Handle<Barrier> h);

};

#endif
