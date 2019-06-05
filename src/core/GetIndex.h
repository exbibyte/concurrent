#ifndef GETINDEX_H
#define GETINDEX_H

//==================================
//
// Summary:      Retrieves index in an ordered sequence of unique types
// Notes:
// Dependencies: N/A
//==================================

namespace GetIndex 
{
    template <typename T, typename... Ts> struct get_index;

    template <typename T, typename... Ts>
    struct get_index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

    template <typename T, typename Tail, typename... Ts>
    struct get_index<T, Tail, Ts...> : std::integral_constant<std::size_t, 1 + get_index<T, Ts...>::value> {};
}

#endif
