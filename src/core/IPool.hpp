#ifndef IPOOL_H
#define IPOOL_H

#include <utility>
#include <functional>

class trait_pool_concurrency {
public:
    class none{};
    class global{};
    class granular{};
    class lockfree{};
    class waitfree{};
};

class trait_pool_size {
public:
    class bounded{};
    class unbounded{};
};

class trait_pool_method {
public:
    class total{};
    class partial{};
    class synchronous{};
};

class trait_pool_fairness {
public:
    class fifo{};
    class lifo{};
};

template< class T, template< class > class ContainerType, class PoolSize, class PoolConcurrency, class PoolMethod, class PoolFairness >
class IPool final : public ContainerType<T> {
public:
        //container and value traits
        using container_type =     ContainerType<T>;
	using value_type =         T;
	using reference =          T &;
	using const_reference =    T const &;
	using size_type =          typename ContainerType<T>::_t_size;

        //pool traits
        using pool_size =          PoolSize;
	using pool_concurrency =   PoolConcurrency;
	using pool_method =        PoolMethod;
        using pool_fairness =      PoolFairness;

              template< class... Args >
              IPool( Args... args ) : ContainerType<T>( std::forward<Args>(args)... ) {}
              ~IPool(){}
    
         bool clear(){ return ContainerType<T>::clear(); }
         bool empty(){ return ContainerType<T>::empty(); }
    size_type size(){ return ContainerType<T>::size(); }
         bool put( const_reference item ){ return ContainerType<T>::put( item ); }
         bool get( reference item ){ return ContainerType<T>::get( item ); }
         void get_fun( std::function<void(bool,reference)> f ){
	     value_type val;
	     bool ret = get( val );
	     f( ret, val );
	 }
};

#endif
