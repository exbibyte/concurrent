#ifndef IPOOL_HPP
#define IPOOL_HPP

#include <utility>
#include <functional>
#include <type_traits>
#include <optional>

#include "IReclamation.hpp"
#include "IConcurrency.hpp"
#include "ISize.hpp"

template< class T, template< class, trait_reclamation > class ContainerType, trait_size pool_size_, trait_concurrency pool_concurrency_, trait_method pool_method_, trait_fairness pool_fairness_, trait_reclamation reclam >
class IPool final : public ContainerType<T, reclam> {
public:

    static_assert(std::is_move_constructible<T>::value);
    static_assert(std::is_move_assignable<T>::value);
    
        //container and value traits
    using container_type =     ContainerType<T, reclam>;
    using value_type =         T;
    using reference =          T &;
    using const_reference =    T const &;
    using size_type =          typename container_type::_t_size;

        //pool traits
        constexpr static trait_size pool_size = pool_size_;
        constexpr static trait_concurrency pool_concurrency = pool_concurrency_;
        constexpr static trait_method pool_method = pool_method_;
        constexpr static trait_fairness pool_fairness = pool_fairness_;
        constexpr static trait_reclamation pool_reclamation = reclam;

              template< class... Args >
              IPool( Args&& ... args ) : container_type( std::forward<Args>(args)... ) {}
              ~IPool(){}
    
         bool clear(){ return container_type::clear(); }
         bool empty(){ return container_type::empty(); }
    size_type size(){ return container_type::size(); }
         template< class _T >
         bool put( _T&& item ){
             return container_type::put( std::forward<_T>(item));
         }
         bool put_with( std::function<value_type()> f ){
             return put( f() );
         }
         std::optional<value_type> get(){ return container_type::get(); }
         void get_with( std::function<void(std::optional<value_type>)> f ){
             f( get() );
         }
};

#endif
