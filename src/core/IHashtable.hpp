#ifndef IHASHTABLE_HPP
#define IHASHTABLE_HPP

#include <vector>
#include <iterator>
#include <functional>
#include <type_traits>
#include <random>
#include <cmath>
#include <iostream>

class trait_hashtable_hashing_method {
public:
    class division{};
    class multiplication{};
    class universal{};
    class other{};
};

class trait_hashtable_method {
public:
    class closed{};
    class open{};
};

class trait_hashtable_concurrency {
public:
    class none{};
    class global{};
    class granular{};
    class disjoint_access_parallelism{};
    class lockless{};
    class waitfree{};
};

class trait_hashtable_lock_load_factor {
public:
    class constant{};
    class linear{};
    class not_applicable{};
};

template< class K, class V, template<class,class> class Impl, class HtConcurrency, class HtMethod, class HtLockLoadFactor, class HashMethod >
class IHashtable final : public Impl< K, V > {
public:
         using key_type =               K;
         using key_const_type =         K const;
         using value_type =             V;
         using val_reference =          V &;
         using val_const_reference =    V const &;

	 //hashtable traits
	 using hashtable_impl = Impl< K, V >;
	 using hashtable_concurrency = HtConcurrency;
	 using hashtable_method = HtMethod;
	 using hashtable_lock_load_factor = HtLockLoadFactor;
	 using hashtable_hash_method = HashMethod;
	 
               template< class... Args >
               IHashtable( Args... args ) : Impl< K, V >( std::forward<Args>( args )... ) {}
               ~IHashtable(){}

          bool insert( key_const_type k, val_const_reference v ){ return Impl< K, V >::insert( k, v ); }
          bool find( key_const_type k, val_reference v ){ return Impl< K, V >::find( k, v ); }
          bool erase( key_const_type k ){ return Impl< K, V >::erase( k ); }
          bool add_hash_func( std::function<size_t(size_t)> f ){ return Impl< K, V >::add_hash_func( f ); }
          bool get_hash_func_current( std::function<size_t(size_t)> & f ){ return Impl< K, V >::get_hash_func_current( f ); }
          bool resize( size_t size ){ return Impl< K, V >::resize( size ); }
        size_t get_table_size(){ return Impl< K, V >::get_table_size(); }
        double get_load_factor(){ return Impl< K, V >::get_load_factor(); }
};

#endif
