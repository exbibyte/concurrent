#ifndef ICOLLECTOR_HPP
#define ICOLLECTOR_HPP

class trait_collector_concurrency {
public:
    class none{};
    class global{};
    class granular{};
    class lockfree{};
    class waitfree{};
};

class trait_collector_method {
public:
    class copy{};
    class compact{};
    class sweep{};
};

class trait_collector_detection {
    class refcount{};
    class tracing{};
    class bitmap{};
};
    
class trait_collector_partition_space {
public:
    class none{};
    class size{};
    class type{};
};

class trait_collector_partition_time {
public:
    class none{};
    class generational{};
};

template< class Impl, class CollectorConcurrency, class CollectorMethod, class CollectorDetection, class CollectorPartitionSpace, class CollectorPartitionTime >
class ICollector final : public Impl {
public:
    //collector traits
    using collector_impl = Impl;
    using collector_concurrency = CollectorConcurrency;
    using collector_method = CollectorMethod;
    using collector_detection = CollectorDetection;
    using collector_partition_space = CollectorPartitionSpace;
    using collector_partition_time = CollectorPartitionTime;

    template< class... Args >
    ICollector( Args... args ) : Impl( std::forward<Args>(args)... ) {}
    ~ICollector(){}
    template< class... Args >
    bool free( Args... args ){ return Impl::free( std::forward<Args>(args)... ); }

    //internal helpers
    template< class... Args >
    bool add_internal( Args... args ){ return Impl::resize_internal( std::forward<Args>(args)... ); }
    template< class... Args >
    bool remove_internal( Args... args ){ return Impl::remove_internal( std::forward<Args>(args)... ); }
    template< class... Args >
    bool reset_internal( Args... args ){ return Impl::reset_internal( std::forward<Args>(args)... ); }
};

#endif
