#ifndef __ORBIT_SCENE__
#define __ORBIT_SCENE__

#include <SDL3/SDL.h>
#include <entt/entt.hpp>

template<typename T>
class SDLForwardAllocator
{
public:
    using value_type = T;
    
    template<typename U>
    constexpr SDLForwardAllocator(const SDLForwardAllocator<U>&) noexcept {}
    
    constexpr SDLForwardAllocator() noexcept = default;
    
    [[nodiscard]] T* allocate(std::size_t n)
    {
        auto* ptr = SDL_malloc(n * sizeof(T));

        if (!ptr)
            throw std::bad_alloc{};

        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, std::size_t) noexcept
    {
        SDL_free(ptr);
    }
};

template<typename T, typename U>
constexpr bool operator==(
    const SDLForwardAllocator<T>&,
    const SDLForwardAllocator<U>&) noexcept
{
    return true;
}

using Registry = entt::basic_registry<entt::entity, SDLForwardAllocator<entt::entity>>;

#endif
