#include <app/app.hpp>

using namespace Orbit;

App::Status App::startup()
{
    return Status::CONTINUE;
}


App::Status App::update(u64 tick)
{
    return Status::CONTINUE;
}

void App::shutdown()
{
}
