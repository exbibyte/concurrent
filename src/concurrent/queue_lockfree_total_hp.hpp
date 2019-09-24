#ifndef QUEUE_LF_TOTAL_HP_HPP
#define QUEUE_LF_TOTAL_HP_HPP

template< class T >
class queue_lockfree_total_impl< T, trait_reclamation::hp > {
public:
    //A value of type T that a node holds is assumed to be default constructable
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
              };

               queue_lockfree_total_impl();
               ~queue_lockfree_total_impl();
   static void thread_deinit(); //to be called when a thread exits
          bool clear();
          bool empty();
       _t_size size(); //approximate count of the container size
          bool put( _t_val const & val ){ return push_back( val ); }
          bool get( _t_val & val ){ return pop_front( val ); }
private:
          bool push_back( _t_val const & val );
          bool pop_front( _t_val & val );
       _t_node _head;
       _t_node _tail;
};

#include "queue_lockfree_total_hp.tpp"

#endif
