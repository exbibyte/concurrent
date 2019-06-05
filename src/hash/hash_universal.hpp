#ifndef HASH_UNIVERSAL_HPP
#define HASH_UNIVERSAL_HPP

#include <functional>
#include <vector>
#include <cstring>

template< class T >
class hash_universal{
public:
    static bool generate( size_t const table_size, std::vector< std::function< size_t( size_t ) > > & funcs );
};

#include "hash_universal.tpp"

#endif
