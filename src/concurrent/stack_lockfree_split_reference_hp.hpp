//implementation of split reference lock free stack based on C++ Concurrency in Action Section 7.2
#ifndef STACK_LF_SPLIT_REF_HP_HPP
#define STACK_LF_SPLIT_REF_HP_HPP

#include <atomic>
#include <cstddef>
#include <mutex>

#include "reclam_hazard.hpp"

template< class T >
class stack_lockfree_split_reference_impl< T, trait_reclamation::hp > {
public:

    static_assert(std::is_move_constructible<T>::value);
    
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

    using mem_reclam = reclam_hazard<NodeExternal>;

    class Node {
    public:
        std::atomic<int> _count_internal;
                       T _val;
          NodeExternal * _next;
                         Node( T const & val ) : _val( val ), _next( nullptr ), _count_internal(0) {}
                         Node( T && val ) : _val( val ), _next( nullptr ), _count_internal(0) {}
    };
    
                                  stack_lockfree_split_reference_impl() : _head( nullptr ) {}
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

#include "stack_lockfree_split_reference_hp.tpp"

#endif
