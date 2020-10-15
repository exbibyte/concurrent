//implementation of split reference lock free stack based on C++ Concurrency in Action Section 7.2
#ifndef STACK_LF_SPLIT_REF_HPP
#define STACK_LF_SPLIT_REF_HPP

#include <atomic>
#include <cstddef>
#include <mutex>

#include "IPool.hpp"

//todo: specialize for reclamation

template< class T, trait_reclamation reclam >
class stack_lockfree_split_reference_impl {
public:
    using _t_size = size_t;
    using _t_val = T;
    using maybe = std::optional<T>;
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
                         Node( T const & val ) : _val( val ), _next( nullptr ), _count_internal(0) {}
                         Node( T && val ) : _val( val ), _next( nullptr ), _count_internal(0) {}
    };
    
                                  stack_lockfree_split_reference_impl() : _head( nullptr ) {}
                      static void thread_init(){}
                      static void thread_deinit(){}
                           size_t size() const; //not guaranteed to be consistent when threads are accessing stack
                             bool put( T const & val ){ return push( val ); }
                             bool put( T && val ){ return push( val ); }
                            maybe get(){ return pop(); }
private:
                             bool push( T const & val );
                             bool push( T && val );
                             bool push_aux( NodeExternal * );
                            maybe pop();

                             bool AcquireNode( NodeExternal * & );
    std::atomic< NodeExternal * > _head;
};

#include "stack_lockfree_split_reference.tpp"

template< class T, trait_reclamation reclam >
using stack_lockfree_split_reference = IPool< T, stack_lockfree_split_reference_impl,
					      trait_size::unbounded,
					      trait_concurrency::lockfree,
					      trait_method::partial,
					      trait_fairness::lifo,
                          reclam >;

#endif
