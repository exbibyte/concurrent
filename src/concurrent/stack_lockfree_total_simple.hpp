#ifndef STACK_LF_TOTAL_SIMPLE_H
#define STACK_LF_TOTAL_SIMPLE_H

#include <atomic>
#include <cstddef>

#include "IPool.hpp"

template< class T >
class stack_lockfree_total_simple_impl {
public:
    using _t_size = size_t;
    using _t_val = T;
    class Node;
    using _t_node = std::atomic< Node * >;
              class Node {
              public:
                          T _val;
                     Node * _next;
                            Node(): _next( nullptr ) {}
                            Node( T const & val ) : _val( val ), _next( nullptr ) {}
              };
              stack_lockfree_total_simple_impl();
              ~stack_lockfree_total_simple_impl();
         bool clear();
         bool empty() const;
      _t_size size() const;
         bool put( T const & val ){ return push( val ); }
         bool get( T & val ){ return pop( val ); }
private:
         bool push( T const & val );
         bool pop( T & val );
      _t_node _head;
};

#include "stack_lockfree_total_simple.tpp"

template< class T >
using stack_lockfree_total_simple = IPool< T, stack_lockfree_total_simple_impl,
					   trait_pool_size::unbounded,
					   trait_pool_concurrency::lockfree,
					   trait_pool_method::total,
					   trait_pool_fairness::lifo >;
#endif
