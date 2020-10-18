//implementation of split reference lock free stack based on C++ Concurrency in Action Section 7.2
#ifndef STACK_LF_SPLIT_REF_HPP
#define STACK_LF_SPLIT_REF_HPP

#include <atomic>
#include <cstddef>
#include <mutex>

#include "IPool.hpp"
#include "reclam_none.hpp"

template< class T, trait_reclamation reclam >
class stack_lockfree_split_reference_impl {
public:
    using _t_size = size_t;
    using _t_val = T;
    using maybe = std::optional<T>;
    using mem_reclam = reclam_none;
    
                                  stack_lockfree_split_reference_impl(){
                                      assert(false && "unsupported reclamation strategy");
                                  }
                      static void thread_init(){}
                      static void thread_deinit(){}
                           size_t size(){ return 0; }
                             bool put( T const & val ){ return false; }
                             bool put( T && val ){ return false; }
                            maybe get(){ return std::nullopt; }
};

#include "stack_lockfree_split_reference_hp.hpp"

template< class T, trait_reclamation reclam >
using stack_lockfree_split_reference = IPool< T, stack_lockfree_split_reference_impl,
                          trait_size::unbounded,
                          trait_concurrency::lockfree,
                          trait_method::partial,
                          trait_fairness::lifo,
                          reclam >;

#endif
