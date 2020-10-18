#ifndef STACK_LF_ELIM_HP_HPP
#define STACK_LF_ELIM_HP_HPP

#include "IPool.hpp"
#include "stack_lockfree_total_simple.hpp"
#include "elimination_array.hpp"
#include "reclam_hazard.hpp"

#include <atomic>

template< class T >
class stack_lockfree_elim_impl< T, trait_reclamation::hp > {
public:

    static_assert(std::is_move_constructible<T>::value);
    static_assert(std::is_move_assignable<T>::value);
    
    using _t_size = size_t;
    using _t_val = T;
    using maybe = std::optional<T>;
    using internal_stack = stack_lockfree_total_simple_impl<T, trait_reclamation::hp>;
    using internal_node = typename internal_stack::Node;
    using mem_reclam = reclam_hazard<internal_node>;

              stack_lockfree_elim_impl(size_t array_size = 2) : _elim_array(array_size) {}
              ~stack_lockfree_elim_impl(){}
  static void thread_init(){ internal_stack::thread_init(); }
  static void thread_deinit(){ internal_stack::thread_deinit(); }
         bool clear(){ return _stack.clear(); }
         bool empty(){ return _stack.empty(); }
       size_t size(){ return _stack.size(); }
         bool put( T && val );
         bool put( T const & val );
        maybe get();

         bool check_elimination(){
             
             std::cout <<"elim count: " << eliminated.load() << ", " << eliminated_2.load() << std::endl;
             return eliminated.load()==eliminated_2.load();
         }
private:
         bool put_aux( internal_node * );
    
    stack_lockfree_total_simple_impl<T, trait_reclamation::hp> _stack;
    elimination_array<internal_node *> _elim_array;

    static constexpr bool debug = false;
    std::atomic<int> eliminated = 0;
    std::atomic<int> eliminated_2 = 0;
};

#include "stack_lockfree_elim_hp.tpp"

#endif
