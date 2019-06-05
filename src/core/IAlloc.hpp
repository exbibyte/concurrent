#ifndef IALLOC_HPP
#define IALLOC_HPP

class trait_alloc_concurrency {
public:
    class none{};
    class global{};
    class granular{};
    class lockfree{};
    class waitfree{};
};

class trait_alloc_method {
public:
    class first_fit{};
    class next_fit{};
    class best_fit{};
};

class trait_alloc_partition {
public:
    class none{};
    class size{};
    class type{};
};

template< class Impl, class AllocConcurrency, class AllocMethod, class AllocPartition >
class IAlloc final : Impl {
public:
    //allocator traits
    using alloc_impl = Impl;
    using alloc_concurrency = AllocConcurrency;
    using alloc_method = AllocMethod;
    using alloc_partition = AllocPartition;

    template< class... Args >
    IAlloc( Args&&... args ) : Impl( std::forward<Args>(args)... ) {}
    ~IAlloc(){}
    //lend out a free block to user
    template< class... Args >
    bool allocating( Args&&... args ){ return Impl::allocating( std::forward<Args>(args)... ); }
    //reclaim a block. todo: to be relocated to responsibility of specific garbage collector
    template< class... Args >
    bool freeing( Args&&... args ){ return Impl::freeing( std::forward<Args>(args)... ); }
    //allocate and initializes T if p is nullptr, else initialize T using existing placement p
    template< class T, class... Args >
    T * newing( void * p, Args&&... args ){ return Impl::template newing<T,Args...>( p, std::forward<Args>(args)... ); }
    //todo: to be relocated to responsibility of specific garbbage collector
    template< class T >
    bool deleting( T * p ){ return Impl::deleting( p ); }
	 
    size_t stat_free_size_total(){ return Impl::stat_free_size_total(); }
    size_t stat_free_size_largest(){ return Impl::stat_free_size_largest(); }
    double stat_free_size_mean(){ return Impl::stat_free_size_mean(); }
    size_t stat_free_count_blocks(){ return Impl::stat_free_count_blocks(); }
    double stat_free_fraction(){ return Impl::stat_free_fraction(); }

    size_t stat_lent_size_total(){ return Impl::stat_lent_size_total(); }
    double stat_lent_size_mean(){ return Impl::stat_lent_size_mean(); }
    size_t stat_lent_count_blocks(){ return Impl::stat_lent_count_blocks(); }
    
    //internal helpers
    //remove currently owned block(s) and repoint to a free block elsewhere
    template< class... Args >
    bool resize_internal( Args... args ){ return Impl::resize_internal( std::forward<Args>(args)... ); }
    bool clear_internal(){ return Impl::clear_internal(); }
};

#endif
