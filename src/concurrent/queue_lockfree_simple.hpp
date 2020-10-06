//--unbounded lock free simple queue
//---based on Art of Multiprocessor Programming section 10.5
//----not ABA safe
//----A value of type T that a Node<T> holds is assumed to be
//------default constructable and copy assignable
//----allocation and deallocation is not taken care of inside the datastructure/algorithm
#ifndef QUEUE_LF_SIMPLE_HPP
#define QUEUE_LF_SIMPLE_HPP

#include <cstring>
#include <atomic>
#include <functional>

template< class T >
class queue_lockfree_simple_impl {
public:

    static_assert(std::is_move_constructible<T>::value);
    
    using _t_size = size_t;
    using _t_val = T;
    class Node;
    using _t_node = std::atomic< Node * >;

              class Node {
              public:
                     _t_node _next;
                      _t_val _val;
		             Node(): _next( nullptr ) {}
		             Node( _t_val const & val ): _val(val), _next( nullptr ) {}
                     Node( _t_val && val ): _val(val), _next( nullptr ) {}
	      };

               queue_lockfree_simple_impl();
               ~queue_lockfree_simple_impl();

          bool clear();
          bool empty();
       _t_size size();                                                 //approximate count of the container size
          bool push_back( Node * const );
          bool pop_front( Node * & );
          void for_each( std::function<void(Node * const)> );
private:
       _t_node _head;
       _t_node _tail;
};

#include "queue_lockfree_simple.tpp"

template< class T >
using queue_lockfree_simple = queue_lockfree_simple_impl<T>;

#endif
