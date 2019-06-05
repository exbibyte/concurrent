//implementation of split reference lock free stack based on C++ Concurrency in Action Section 7.2
#ifndef STACK_LF_SPLIT_REF_H
#define STACK_LF_SPLIT_REF_H

#include <atomic>
#include <cstddef>
#include <mutex>

#include "IPool.hpp"

template< class T >
class stack_lockfree_split_reference_impl {
public:
    using _t_size = size_t;
    using _t_val = T;
    class Node;
    class NodeExternal {
    public:
	int _count_external;
	Node * _node;
	NodeExternal() : _node( nullptr ), _count_external(0) {}
    };
    class Node {
    public:
	std::atomic<int> _count_internal;
	T _val;
	NodeExternal * _next;
	Node( T val ) : _val( val ), _next( nullptr ), _count_internal(0) {}
    };
    stack_lockfree_split_reference_impl() : _head( nullptr ) {}
    size_t size() const; //not guaranteed to be consistent when threads are accessing stack
    bool put( T const & val ){ return push( val ); }
    bool get( T & val ){ return pop( val ); }
private:
    bool push( T const & val );
    bool pop( T & val );

    bool AcquireNode( NodeExternal * & );
    std::atomic< NodeExternal * > _head;
};

#include "stack_lockfree_split_reference.tpp"

template< class T >
using stack_lockfree_split_reference = IPool< T, stack_lockfree_split_reference_impl,
					      trait_pool_size::unbounded,
					      trait_pool_concurrency::lockfree,
					      trait_pool_method::partial,
					      trait_pool_fairness::lifo >;

#endif
