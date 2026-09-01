#include <platform/platform.hpp>

#include <SDL3/SDL.h>

using namespace Orbit;

int THREAD(void *)
{
    printf("Threading... ");
    
    return 0;
}

int main(void)
{
    Platform::init();

    Handle<Platform::Window> window = Platform::create_window({});
    Handle<Platform::Mutex> mutex = Platform::create_mutex();
    Handle<Platform::RWlock> rwlock = Platform::create_rwlock();
    
    Handle<Platform::Barrier> barrier = Platform::create_barrier(std::thread::hardware_concurrency());
    
    Handle<Platform::Thread> thread = Platform::create_thread({.name = "antonis", .funct = THREAD, .data = NULL});

    u8 running = 1;
    while (running)
    {
        SDL_Event event;
        SDL_PollEvent(&event);

        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                running = 0;
                break;

            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                    running = 0;
                break;
        }
    }
    
    Platform::wait_thread(thread);
    
    Platform::destroy();
    return 0;
}
