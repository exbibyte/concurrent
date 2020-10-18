#ifndef QUEUE_LF_TOTAL_EPOCH_HPP
#define QUEUE_LF_TOTAL_EPOCH_HPP

#include "reclam_epoch.hpp"

template< class T >
class queue_lockfree_total_impl< T, trait_reclamation::epoch > {
public:
    
    static_assert(std::is_move_constructible<T>::value);

    using _t_size = size_t;
    using _t_val = T;
    class Node;
    using _t_node = std::atomic< Node * >;
    using maybe = std::optional<_t_val>;
              class Node {
              public:
                     _t_node _next;
                      _t_val _val;
                             Node(): _next( nullptr ) {}
                             Node( _t_val const & val ): _val(val), _next( nullptr ) {}
                             Node( _t_val && val ): _val(val), _next( nullptr ) {}
              };
    using mem_reclam = reclam_epoch<Node>;
    
               queue_lockfree_total_impl();
               ~queue_lockfree_total_impl();
          bool clear();
          bool empty();
       _t_size size(); //approximate count of the container size
          bool put( _t_val && val ){ return push_back( val ); }
          bool put( _t_val const & val ){ return push_back( val ); }
         maybe get(){ return pop_front(); }
private:
          bool push_back( _t_val const & val );
          bool push_back( _t_val && val );
          bool push_back_aux( Node * );
         maybe pop_front();
       _t_node _head;
       _t_node _tail;
};

#include "queue_lockfree_total_epoch.tpp"

#endif
