#include <SDL3/SDL.h>
#include <window.h>

using namespace Orbit;

int main(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("Failed to initialized!");
        return -1;
    }

    {
        WindowDesc desc{.props = WindowFlags::INPUT_FOCUS |
                                 WindowFlags::MOUSE_FOCUS |
                                 WindowFlags::RESIZABLE};
        Window window(desc);

        std::uint8_t running = 1;
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
    }

    SDL_Quit();
    return 0;
}
