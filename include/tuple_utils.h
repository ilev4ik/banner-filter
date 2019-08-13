#pragma once
#ifndef RAMBLER_BANNERS_TUPLE_UTILS_H
#define RAMBLER_BANNERS_TUPLE_UTILS_H

#include <tuple>
#include <cstddef>

namespace lvn
{
    template<typename T, T... I>
    struct integer_sequence
    {
        typedef T value_type;
        static constexpr std::size_t size() { return sizeof...(I); }
    };

    template<std::size_t... I>
    using index_sequence = integer_sequence<std::size_t, I...>;

    template<typename T, std::size_t N, T... I>
    struct make_integer_sequence : make_integer_sequence<T, N-1, N-1, I...> {};

    template<typename T, T... I>
    struct make_integer_sequence<T, 0, I...> : integer_sequence<T, I...> {};

    template<std::size_t N>
    using make_index_sequence = make_integer_sequence<std::size_t, N>;

    template<typename... T>
    using index_sequence_for = make_index_sequence<sizeof...(T)>;
}

template<typename... T1, typename... T2, std::size_t... I>
constexpr auto add(const std::tuple<T1...>& t1, const std::tuple<T2...>& t2,
                   lvn::index_sequence<I...>) -> std::tuple<T1...>
{
    return std::tuple<T1...>{ std::get<I>(t1) + std::get<I>(t2)... };
}

template<typename... T1, typename... T2>
constexpr auto operator+(const std::tuple<T1...>& t1, const std::tuple<T2...>& t2) -> std::tuple<T1...>
{
    // make sure both tuples have the same size
    static_assert(sizeof...(T1) == sizeof...(T2), "");

    return add(t1, t2, lvn::make_index_sequence<sizeof...(T1)>{});
}

#endif //RAMBLER_BANNERS_TUPLE_UTILS_H
