#ifndef __ORBIT_WINDOW__
#define __ORBIT_WINDOW__

#include <cstdint>

namespace Orbit
{

enum class WindowFlags : std::uint64_t
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
    return static_cast<WindowFlags>(static_cast<std::uint64_t>(lhs) |
                                    static_cast<std::uint64_t>(rhs));
}

constexpr WindowFlags operator&(WindowFlags lhs, WindowFlags rhs) noexcept
{
    return static_cast<WindowFlags>(static_cast<std::uint64_t>(lhs) &
                                    static_cast<std::uint64_t>(rhs));
}

struct WindowDesc
{
    const char *name = "Orbit";
    WindowFlags props = WindowFlags::NONE;
    std::uint64_t width = 1024;
    std::uint64_t height = 720;
};

class Window final
{
public:
    Window() = delete;
    Window(WindowDesc& desc);
    ~Window();

    Window(const Window& other) = delete;
    Window& operator=(const Window& other) = delete;

    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    void *native();

private:
    void *pHandle = nullptr;
};

};

#endif
