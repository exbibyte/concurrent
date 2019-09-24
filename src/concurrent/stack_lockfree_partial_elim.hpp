//--unbounded lock free partial stack
//---based on Art of Multiprocessor Programming section 11.4
#ifndef STACK_LF_PARTIAL_ELIM_H
#define STACK_LF_PARTIAL_ELIM_H

#include "IPool.hpp"
#include <atomic>

template< class T >
class stack_lockfree_partial_elim_impl {
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
              stack_lockfree_partial_elim_impl();
              ~stack_lockfree_partial_elim_impl();
         bool clear();
         bool empty() const;
    _t_size_t size() const; //not guaranteed to be consistent when threads are accesing stack
         bool put( T const & val ){ return push( val ); }
         bool get( T & val ){ return pop( val ); }
private:
         bool push( T const & val );
         bool pop( T & val );
      _t_node _head;
};

#include "stack_lockfree_partial_elim.tpp"

template< class T >
using stack_lockfree_partial_elim = IPool< T, stack_lockfree_partial_elim_impl,
					   trait_pool_size::unbounded,
					   trait_pool_concurrency::lockfree,
					   trait_pool_method::partial,
					   trait_pool_fairness::lifo,
					   trait_pool_reclamation::not_applicable >;

#endif
