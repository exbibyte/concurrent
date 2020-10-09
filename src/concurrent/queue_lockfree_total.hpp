//--unbounded lock free total queue
//---uses hazard pointer memory reclamation

#ifndef QUEUE_LF_TOTAL_HPP
#define QUEUE_LF_TOTAL_HPP

#include <cstring>
#include <atomic>
#include "IPool.hpp"

template< class T, trait_reclamation reclam >
class queue_lockfree_total_impl {
public:

    using _t_size = size_t;
    using _t_val = T;
    using maybe = std::optional<_t_val>;
               queue_lockfree_total_impl(){
                   assert(false && "unsupported reclamation strategy");
               }
               ~queue_lockfree_total_impl(){}
    static void thread_init(){}
    static void thread_deinit(){}
          bool clear(){return true;}
          bool empty(){return true;}
       _t_size size(){ return 0; }
          bool put( _t_val && val ){ return false; }
          bool put( _t_val const & val ){ return false; }
         maybe get(){ return std::nullopt; }
};

//specializations for different memory reclamations:

#include "queue_lockfree_total_hp.hpp"
#include "queue_lockfree_total_epoch.hpp"

template< class T, trait_reclamation reclam >
using queue_lockfree_total = IPool< T, queue_lockfree_total_impl,
				    trait_size::unbounded,
				    trait_concurrency::lockfree,
				    trait_method::total,
				    trait_fairness::fifo,
				    reclam >;

#endif
