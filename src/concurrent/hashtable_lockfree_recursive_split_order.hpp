#ifndef IHASHTABLE_LOCKFREE_RECURSIVE_SPLIT_ORDER_HPP
#define IHASHTABLE_LOCKFREE_RECURSIVE_SPLIT_ORDER_HPP

#include <vector>
#include <iterator>
#include <functional>
#include <type_traits>
#include <random>
#include <cmath>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>

#include "IHashtable.hpp"
#include "lock_recursive_block.hpp"

template< class K, class V >
class hashtable_lockfree_recursive_split_order_impl {
public:
                  hashtable_lockfree_recursive_split_order_impl( size_t table_size, double lock_factor );
                  ~hashtable_lockfree_recursive_split_order_impl();

             bool insert( K const, V const & );
             bool find( K const , V & );
             bool erase( K const );
             bool add_hash_func( std::function< size_t( size_t ) > );
             bool get_hash_func_current( std::function< size_t( size_t ) > & );
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
            using vec_lock =         std::vector< lock_recursive_block >;

             bool compute_hash( K const key, size_t & hashed_val );
             bool select_random_hash_func();
             bool set_default_hash_funcs( size_t const table_size );

       hashnode * prepend_hashnode( hashnode * node, K const key, V const & val );
             bool remove_hashnode( hashnode * & node );
       hashnode * find_hashnode( hashnode * const node, K const key );
public:
         vec_lock _locks;
     vec_hashnode _table;
    vec_hash_func _funcs_hash;
        hash_func _func_hash_selected;
std::atomic<size_t> _count_items;
           double _lock_factor;
std::atomic<size_t> _lock_count;
};

#include "hashtable_lockfree_recursive_split_order.tpp"

template< class K, class V >
using hashtable_lockfree_recursive_split_order = IHashtable< K, V, hashtable_lockfree_recursive_split_order_impl,
					      trait_hashtable_concurrency::lockless,
					      trait_hashtable_method::closed,
					      trait_hashtable_lock_load_factor::constant,
					      trait_hashtable_hashing_method::universal
					      >;

#endif
