#ifndef HASHTABLE_UNIVERSAL_CHAIN_HPP
#define HASHTABLE_UNIVERSAL_CHAIN_HPP

#include <vector>
#include <iterator>
#include <functional>
#include <type_traits>
#include <random>
#include <cmath>
#include <iostream>

#include "IHashtable.hpp"

template< class K, class V >
class hashtable_universal_chain_impl {
public:
                  hashtable_universal_chain_impl( size_t table_size );
                  ~hashtable_universal_chain_impl();

             bool insert( K const, V const & );
             bool find( K const , V & );
             bool erase( K const );
             bool add_hash_func( std::function< size_t( size_t ) > ); //add hash function into the univeral set
             bool get_hash_func_current( std::function< size_t( size_t ) > & ); //add hash function into the univeral set
             bool resize( size_t size );
           size_t get_table_size();
           double get_load_factor();
private:
                  class hashnode {
		  public:		      
		      K          _key;
		      hashnode * _next = nullptr;
		      hashnode * _prev = nullptr;
		      V          _val;
		  };

            using vec_hash_func =    std::vector< std::function< size_t( size_t )> >;
            using vec_hashnode =     std::vector< hashnode * >;
            using hash_func =        std::function< size_t( size_t ) >;

             bool compute_hash( K const key, size_t & hashed_val );
             bool select_random_hash_func();
             bool set_default_hash_funcs( size_t const table_size );
             bool prepend_hashnode( hashnode * & node, K const key, V const & val );
             bool remove_hashnode( hashnode * & node );
       hashnode * find_hashnode( hashnode * const node, K const key );

     vec_hashnode _table;
    vec_hash_func _funcs_hash;
        hash_func _func_hash_selected;
           size_t _count_items;
};

#include "hashtable_universal_chain.tpp"

template< class K, class V >
using hashtable_universal_chain = IHashtable< K, V, hashtable_universal_chain_impl,
					      trait_hashtable_concurrency::none,
					      trait_hashtable_method::closed,
					      trait_hashtable_lock_load_factor::constant,
					      trait_hashtable_hashing_method::universal
					      >;

#endif
