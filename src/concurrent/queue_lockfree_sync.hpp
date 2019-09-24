//--unbounded lock free synchronous queue via dual structures
//---based on Art of Multiprocessor Programming section 10.7
#ifndef QUEUE_LF_SYNC_HPP
#define QUEUE_LF_SYNC_HPP

#include <atomic>
#include "IPool.hpp"

//todo: specialize for memory reclamation strategies

//A value of type T that a node holds is assumed to be default constructable
template< class T, trait_reclamation reclam >
class queue_lockfree_sync_impl {
public:
    using _t_size = size_t;
    using _t_val = T;
    class Node;
    using _t_node = std::atomic< Node * >;
    enum class NodeType;
    using _t_node_type = std::atomic< NodeType >;
              enum class NodeType{
                  SENTINEL,
                  ITEM,
                  RESERVATION,
		  BUSY,
		  FULFILLED,
		  COMPLETE,    
              };
              class Node {
              public:
                     _t_node _next;
		      _t_val _val; //container value storage
		_t_node_type _type; //node type( ITEM: enquing thread waiting for synchronization, RESERVATION: dequing thread waiting for synchronization )
		             Node(): _next( nullptr ) {
				 _type.store( NodeType::RESERVATION );
                             }
                             Node( _t_val const & val ): _val( val ), _next( nullptr ){
				 _type.store( NodeType::ITEM );
                             }
              };

               queue_lockfree_sync_impl();
               ~queue_lockfree_sync_impl();
          bool clear();
          bool empty();
       _t_size size();                                                 //approximate count of the container size
          bool put( _t_val const & val ){ return push_back( val ); }
          bool get( _t_val & val ){ return pop_front( val ); }
private:
          bool push_back( _t_val const & val );
          bool pop_front( _t_val & val );
       _t_node _head;
       _t_node _tail;
};

#include "queue_lockfree_sync.tpp"

template< class T, trait_reclamation reclam >
using queue_lockfree_sync = IPool< T, queue_lockfree_sync_impl,
				   trait_size::unbounded,
				   trait_concurrency::lockfree,
				   trait_method::synchronous,
				   trait_fairness::fifo,
				   reclam >;

#endif

