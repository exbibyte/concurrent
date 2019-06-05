#ifndef ELIMINATION_ARRAY_HPP
#define ELIMINATION_ARRAY_HPP

#include <array>
#include <random>
#include <cstring>

#include "exchanger_lockfree.hpp"

template<typename ValType, size_t ArraySize>
class elimination_array {
public:
    static_assert( ArraySize >= 1, "ArraySize required to be at least 1" );
    size_t const _size = ArraySize;
    bool visit( ValType & val, size_t range );
    bool set_timeout( long duration_us );
    elimination_array() : _timeout_us( 100000 ) {}
    std::array< exchanger_lockfree<ValType>, ArraySize > _elim_array;
    long _timeout_us;
    std::default_random_engine _generator;

};

#include "elimination_array.tpp"

#endif
