#ifndef ILIST_H
#define ILIST_H

#include <utility>
#include <functional>

class trait_list_concurrency {
public:
    class none{};
    class global{};
    class granular{};
    class lockfree{};
    class waitfree{};
};

class trait_list_size {
public:
    class bounded{};
    class unbounded{};
};

class trait_list_method {
public:
    class total{};
    class partial{};
    class synchronous{};
};

template< class T, template< class > class ContainerType, class ListSize, class ListConcurrency, class ListMethod >
class IList final : public ContainerType<T> {
public:
        //container and value traits
        using container_type =     ContainerType<T>;
	using value_type =         T;
	using reference =          T &;
	using const_reference =    T const &;
	using size_type =          typename ContainerType<T>::_t_size;

        //list traits
        using list_size =          ListSize;
	using list_concurrency =   ListConcurrency;
	using list_method =        ListMethod;

              template< class... Args >
              IList( Args... args ) : ContainerType<T>( std::forward<Args>(args)... ) {}
              ~IList(){}
    
         bool clear(){ return ContainerType<T>::clear(); }
         bool empty(){ return ContainerType<T>::empty(); }
    size_type size(){ return ContainerType<T>::size(); }
         bool add( const_reference item, size_t key ){ return ContainerType<T>::add( item, key ); }
         bool remove( value_type & item, size_t key ){ return ContainerType<T>::remove( item, key ); }
         bool contains( const_reference item, size_t key ){ return ContainerType<T>::contains( item, key ); }
};

#endif
