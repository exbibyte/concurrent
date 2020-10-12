#ifndef STACK_LF_TOTAL_SIMPLE_HPP
#define STACK_LF_TOTAL_SIMPLE_HPP

#include <atomic>
#include <cstddef>
#include <optional>

#include "IPool.hpp"

template< class T, trait_reclamation reclam >
class stack_lockfree_total_simple_impl {
public:
    using _t_size = size_t;
    using _t_val = T;
    using maybe = std::optional<T>;

              stack_lockfree_total_simple_impl(){
                  assert(false && "unsupported reclamation strategy");
              }
              ~stack_lockfree_total_simple_impl(){}
  static void thread_init(){}
  static void thread_deinit(){}
         bool clear(){return true;}
         bool empty(){return true;}
      _t_size size(){ return 0; }
         bool put( T && val ){ return false; }
         bool put( T const & val ){ return false; }
        maybe get(){ return std::nullopt; }
};


#include "stack_lockfree_total_simple_hp.hpp"

template< class T, trait_reclamation reclam >
using stack_lockfree_total_simple = IPool< T, stack_lockfree_total_simple_impl,
					   trait_size::unbounded,
					   trait_concurrency::lockfree,
					   trait_method::total,
					   trait_fairness::lifo,
					   reclam >;
#endif
