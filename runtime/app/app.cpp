#include <app/app.hpp>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

namespace Orbit::App
{

std::atomic<u64> pTick(0);

static SDL_AppResult to_sdl(Status s)
{
    switch (s)
    {
        case Status::SUCCESS: return SDL_APP_SUCCESS;
        case Status::FAILURE: return SDL_APP_FAILURE;
        case Status::CONTINUE:
        default:              return SDL_APP_CONTINUE;
    }
}

}

using namespace Orbit;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    App::Status status = {};

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    status = App::startup();
    if (status != App::Status::CONTINUE) return App::to_sdl(status);


    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;

    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;

    return App::to_sdl(App::event());
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    (void)appstate;

    u64 currTick = App::pTick;
    App::pTick.fetch_add(1);

    return App::to_sdl(App::update(App::pTick - currTick));
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)appstate;
    (void)result;

    SDL_Quit();
}
