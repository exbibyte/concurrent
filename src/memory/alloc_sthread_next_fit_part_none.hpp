#ifndef ALLOC_SINGLE_THREAD_NEXT_FIT_PARTITION_NONE_HPP
#define ALLOC_SINGLE_THREAD_NEXT_FIT_PARTITION_NONE_HPP

#include <cstring>
#include <utility>
#include <functional>
#include <list>

#include "IAlloc.hpp"

class alloc_sthread_next_fit_part_none_impl {
public:
              alloc_sthread_next_fit_part_none_impl( size_t block_size ); //self initialize block from system
              alloc_sthread_next_fit_part_none_impl( void * p, size_t size ); //inherit a block from elsewhere
              ~alloc_sthread_next_fit_part_none_impl();

         bool allocating( void ** p, size_t size, bool zeroed = false );
         bool freeing( void * p );
         template< class T, class... Args >
         T * newing( void * p, Args&&... args );
         template< class T >
	 bool deleting( T * p );

              //stat query
       size_t stat_free_size_total();
       size_t stat_free_size_largest();
       double stat_free_size_mean();
       size_t stat_free_count_blocks();
       double stat_free_fraction();
       size_t stat_lent_size_total();
       double stat_lent_size_mean();
       size_t stat_lent_count_blocks();

              //copy current buffer to an already empty target buffer
         bool resize_internal( void * p, size_t size, bool zeroed = false );
              //copy current buffer to a new empty buffer initialized from system
         bool resize_internal( size_t size );
         bool clear_internal();
private:
              //starting pointer and total size
       void * _p;
       size_t _size;
    
              //free blocks and lent blocks
        using t_list = std::list<std::pair<size_t, size_t> >;
       t_list _alloc_info;
       t_list _lent_info;
};

#include "alloc_sthread_next_fit_part_none.tpp"

using alloc_sthread_next_fit_part_none = IAlloc< alloc_sthread_next_fit_part_none_impl,
						 trait_alloc_concurrency::none,
						 trait_alloc_method::next_fit,
						 trait_alloc_partition::none >;

#endif
