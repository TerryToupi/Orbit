#include <app/app.hpp>
#include <window/window.hpp>

using namespace Orbit;

Handle<Window::Window> win;

App::Status App::startup()
{
    win = Window::create("Orbit", 1024, 720);

    return Status::CONTINUE;
}


App::Status App::update(u64 tick)
{
    return Status::CONTINUE;
}

void App::shutdown()
{
    Window::destroy(win);
}
