#pragma once

#include <cassert>
#include <cstddef>
#include <new>
#include <type_traits>

#include <utils/types.hpp>

namespace Orbit 
{

template<u64 StorageSize>
class FixedFunction
{
public:
    static_assert(StorageSize != 0);

    FixedFunction() noexcept = default;

    FixedFunction(const FixedFunction&) = delete;
    FixedFunction& operator=(const FixedFunction&) = delete;
    FixedFunction(FixedFunction&&) = delete;
    FixedFunction& operator=(FixedFunction&&) = delete;

    template<typename Callback>
    void set(Callback callback) noexcept
    {
        static_assert(std::is_trivially_copyable_v<Callback>);
        static_assert(std::is_trivially_destructible_v<Callback>);
        static_assert(std::is_nothrow_move_constructible_v<Callback>);
        static_assert(std::is_nothrow_invocable_v<Callback&>);
        static_assert(sizeof(Callback) <= StorageSize);
        static_assert(alignof(Callback) <= alignof(std::max_align_t));

        assert(!pInvoke);
        ::new (static_cast<void*>(pStorage)) Callback(static_cast<Callback&&>(callback));
        pInvoke = invoke<Callback>;
    }

    void operator()() noexcept
    {
        assert(pInvoke);
        pInvoke(pStorage);
    }

    void clear() noexcept
    {
        pInvoke = nullptr;
    }

private:
    using Invoke = void (*)(void*) noexcept;

    template<typename Callback>
    static void invoke(void* storage) noexcept
    {
        (*std::launder(reinterpret_cast<Callback*>(storage)))();
    }

    alignas(std::max_align_t) u8 pStorage[StorageSize];
    Invoke pInvoke = nullptr;
};

} // namespace gpu
