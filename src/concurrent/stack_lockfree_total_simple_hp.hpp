#ifndef STACK_LF_TOTAL_SIMPLE_HP_HPP
#define STACK_LF_TOTAL_SIMPLE_HP_HPP

#include <atomic>
#include <cstddef>
#include <optional>

#include "IPool.hpp"
#include "stack_lockfree_elim.hpp"

template< class T>
class stack_lockfree_total_simple_impl< T, trait_reclamation::hp > {
public:

    static_assert(std::is_move_constructible<T>::value);
    
    using _t_size = size_t;
    using _t_val = T;
    class Node;
    using _t_node = std::atomic< Node * >;
    using maybe = std::optional<T>;
              class Node {
              public:
                          T _val;
                     Node * _next;
                            Node(): _next( nullptr ) {}
                            Node( T const & val ) : _val( val ), _next( nullptr ) {}
                            Node( T && val ) : _val( val ), _next( nullptr ) {}
              };
              stack_lockfree_total_simple_impl();
              ~stack_lockfree_total_simple_impl();
  static void thread_init(){}
  static void thread_deinit();
         bool clear();
         bool empty() const;
      _t_size size() const;
         bool put( T && val ){ return push( val ); }
         bool put( T const & val ){ return push( val ); }
        maybe get(){ return pop(); }
         bool try_put( T && val ){ return try_push( val ); }
         bool try_put( T const & val ){ return try_push( val ); }
        maybe try_get(){ return try_pop(); }
private:
         bool push( T const & val );
         bool push( T && val );
         bool push_aux( Node * );
        maybe pop();

         bool try_push( T const & val );
         bool try_push( T && val );
         bool try_push_aux( Node * );
        maybe try_pop();

      _t_node _head;

    friend stack_lockfree_elim_impl< T, trait_reclamation::hp>;
};

#include "stack_lockfree_total_simple_hp.tpp"

#endif
