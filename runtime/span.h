#ifndef ORBIT_SPAN_H
#define ORBIT_SPAN_H

#include <cstddef>
#include <initializer_list>

// Borrows an array; initializer-list storage lasts only through the enclosing full expression.
template<typename T>
struct Span
{
    const T* data = nullptr;
    size_t size = 0;

    constexpr Span() = default;
    constexpr Span(const T* data, size_t size) : data(data), size(size) {}
    constexpr Span(std::initializer_list<T> values) : data(values.begin()), size(values.size()) {}

    template<size_t N>
    constexpr Span(const T (&values)[N]) : data(values), size(N) {}
};

#endif
