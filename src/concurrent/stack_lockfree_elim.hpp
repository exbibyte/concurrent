//--unbounded lock free partial stack
//---based on Art of Multiprocessor Programming section 11.4
#ifndef STACK_LF_ELIM_HPP
#define STACK_LF_ELIM_HPP

#include <atomic>

#include "IPool.hpp"
#include "reclam_none.hpp"

template< class T, trait_reclamation reclam >
class stack_lockfree_elim_impl {
public:
    using _t_val = T;
    using maybe = std::optional<T>;
    using mem_reclaim = reclam_none;
    
              stack_lockfree_elim_impl(){
                  assert(false && "unsupported reclamation strategy");
              }
              ~stack_lockfree_elim_impl(){}
  static void thread_init(){}
  static void thread_deinit(){}
         bool clear(){ return true; }
         bool empty(){ return true; }
       size_t size(){ return 0; }
         bool put( T && val ){ return false; }
         bool put( T const & val ){ return false; }
        maybe get(){ return std::nullopt; }
};

#include "stack_lockfree_elim_hp.hpp"

template< class T, trait_reclamation reclam >
using stack_lockfree_elim = IPool< T, stack_lockfree_elim_impl,
                                   trait_size::unbounded,
                                   trait_concurrency::lockfree,
                                   trait_method::partial,
                                   trait_fairness::lifo,
                                   reclam >;

#endif
