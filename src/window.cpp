#include <type_traits>
#include <window.h>
#include <SDL3/SDL.h>

namespace Orbit
{

template<typename T>
static constexpr std::uint8_t has_flag(T value, T flag)
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


Window::Window(WindowDesc& desc)
{
    SDL_Window *window = SDL_CreateWindow(desc.name,
                                         (int)desc.width,
                                         (int)desc.height,
                                         toSDL(desc.props));
    SDL_assert(window != nullptr);
    pHandle = reinterpret_cast<void*>(window);
}

Window::~Window()
{
   SDL_assert(pHandle);
   SDL_DestroyWindow(reinterpret_cast<SDL_Window*>(pHandle));
}

Window::Window(Window&& other) noexcept
{
    SDL_assert(other.pHandle);
    pHandle = other.pHandle;
    other.pHandle = nullptr;
}

Window& Window::operator=(Window&& other) noexcept
{
    if (this != &other)
    {
        SDL_assert(pHandle);
        SDL_DestroyWindow(reinterpret_cast<SDL_Window*>(pHandle));

        pHandle = other.pHandle;
        other.pHandle = nullptr;
    }

    return *this;
}

void *Window::native()
{
    SDL_assert(pHandle);
    return pHandle;
}

}
