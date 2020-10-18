//--unbounded lock free synchronous queue via dual structures
//---based on Art of Multiprocessor Programming section 10.7
#ifndef QUEUE_LF_SYNC_HP_HPP
#define QUEUE_LF_SYNC_HP_HPP

#include <atomic>
#include "IPool.hpp"
#include "reclam_hazard.hpp"

template< class T >
class queue_lockfree_sync_impl< T, trait_reclamation::hp > {
public:

    static_assert(std::is_move_constructible<T>::value);
    static_assert(std::is_move_assignable<T>::value);
    
    using _t_size = size_t;
    using _t_val = T;
    class Node;
    using _t_node = std::atomic< Node * >;
    enum class NodeType;
    using _t_node_type = std::atomic< NodeType >;
    using maybe = std::optional<T>;
    using mem_reclam = reclam_hazard<Node>;
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
                               Node( _t_val && val ): _val( val ), _next( nullptr ){
                                   _type.store( NodeType::ITEM );
                               }
              };

               queue_lockfree_sync_impl();
               ~queue_lockfree_sync_impl();
   static void thread_init(){}
   static void thread_deinit(){}
          bool clear();
          bool empty();
       _t_size size();                                                 //approximate count of the container size
          bool put( _t_val const & val ){ return push_back( val ); }
          bool put( _t_val && val ){ return push_back( val ); }
         maybe get(){ return pop_front(); }
private:
          bool push_back( _t_val const & val );
          bool push_back( _t_val && val );
          bool push_back_aux( Node * );
         maybe pop_front();
       _t_node _head;
       _t_node _tail;
};

#include "queue_lockfree_sync_hp.tpp"

#endif

