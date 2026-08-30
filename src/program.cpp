#include <SDL3/SDL.h>
#include <platform.hpp>

using namespace Orbit;

int main(void)
{
    Platform::init();

    Handle<Platform::Window> window = Platform::create_window({});
    Handle<Platform::Mutex> mutex = Platform::create_mutex();
    Handle<Platform::RWlock> rwlock = Platform::create_rwlock();
    
    Handle<Platform::Barrier> barrier = Platform::create_barrier(std::thread::hardware_concurrency());

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
    
    Platform::destroy();
    return 0;
}
