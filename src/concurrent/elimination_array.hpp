#ifndef ELIMINATION_ARRAY_HPP
#define ELIMINATION_ARRAY_HPP

#include <array>
#include <random>
#include <cstring>

#include "exchanger_lockfree.hpp"

template<typename ValType, size_t ArraySize, class RandGen = std::minstd_rand>
class elimination_array {
public:
    
    static_assert( ArraySize >= 1, "ArraySize required to be at least 1" );
    
                                            size_t const _size = ArraySize;

                                                         //visit a slot within [start, end)
                                                    bool visit( ValType & val, size_t start, size_t end );
                                                    bool set_timeout( long duration_us );
                                                         elimination_array() : _timeout_us( 100000 ) {}
                                                         elimination_array(RandGen & g) : _timeout_us( 100000 ), _generator(g) {}
                                                         elimination_array(elimination_array const &) = delete;
                                     elimination_array & operator=(elimination_array const &) = delete;
    
    std::array< exchanger_lockfree<ValType>, ArraySize > _elim_array;
                                                    long _timeout_us;
                                                 RandGen _generator;

};

#include "elimination_array.tpp"

#endif
