#ifndef ILIST_HPP
#define ILIST_HPP

#include <utility>
#include <functional>

#include "IReclamation.hpp"
#include "IConcurrency.hpp"
#include "ISize.hpp"

template< class T, template< class, trait_reclamation > class ContainerType, trait_size list_size_, trait_concurrency list_concurrency_, trait_method list_method_, trait_reclamation reclam >
class IList final : public ContainerType<T, reclam> {
public:
        //container and value traits
        using container_type =     ContainerType<T, reclam>;
	using value_type =         T;
	using reference =          T &;
	using const_reference =    T const &;
	using size_type =          typename container_type::_t_size;

        //list traits
        constexpr static trait_size list_size = list_size_;
	constexpr static trait_concurrency list_concurrency = list_concurrency_;
	constexpr static trait_method list_method = list_method_;
        constexpr static trait_reclamation list_reclamation = reclam;
    
              template< class... Args >
              IList( Args... args ) : container_type( std::forward<Args>(args)... ) {}
              ~IList(){}
    
         bool clear(){ return container_type::clear(); }
         bool empty(){ return container_type::empty(); }
    size_type size(){ return container_type::size(); }
         bool add( const_reference item, size_t key ){ return container_type::add( item, key ); }
         bool remove( value_type & item, size_t key ){ return container_type::remove( item, key ); }
         bool contains( const_reference item, size_t key ){ return container_type::contains( item, key ); }
};

#endif
