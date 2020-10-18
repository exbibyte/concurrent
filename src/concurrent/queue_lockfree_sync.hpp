//--unbounded lock free synchronous queue via dual structures
//---based on Art of Multiprocessor Programming section 10.7
#ifndef QUEUE_LF_SYNC_HPP
#define QUEUE_LF_SYNC_HPP

#include <atomic>
#include "IPool.hpp"
#include "reclam_none.hpp"

//todo: specialize for memory reclamation strategies

//A value of type T that a node holds is assumed to be default constructable
template< class T, trait_reclamation reclam >
class queue_lockfree_sync_impl {
public:
    using _t_size = size_t;
    using _t_val = T;
    using maybe = std::optional<T>;
    using mem_reclam = reclam_none;
               queue_lockfree_sync_impl(){
                   assert(false && "unsupported reclamation strategy");
               }
               ~queue_lockfree_sync_impl(){}
   static void thread_init(){}
   static void thread_deinit(){}
          bool clear(){ return true; }
          bool empty(){ return true; }
       _t_size size(){ return 0; }
          bool put( _t_val const & val ){ return false; }
          bool put( _t_val && val ){ return false; }
         maybe get(){ return std::nullopt; }
};

#include "queue_lockfree_sync_hp.hpp"

template< class T, trait_reclamation reclam >
using queue_lockfree_sync = IPool< T, queue_lockfree_sync_impl,
				   trait_size::unbounded,
				   trait_concurrency::lockfree,
				   trait_method::synchronous,
				   trait_fairness::fifo,
				   reclam >;

#endif

