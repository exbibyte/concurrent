//Deprecated. See Funwrap3.
#ifndef FUNCWRAP_H
#define FUNCWRAP_H

#include <memory>
#include <type_traits>
#include <utility>
#include <tuple>
#include <functional>

//unpacking tuple solution from http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer
template<int ...> struct seq {};

template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};

template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };

// template <typename ...Args>
// struct save_it_for_later
// {
//   std::tuple<Args...> params;
//   double (*func)(Args...);

//   double delayed_dispatch()
//   {
//     return callFunc(typename gens<sizeof...(Args)>::type());
//   }

//   template<int ...S>
//   double callFunc(seq<S...>)
//   {
//     return func(std::get<S>(params) ...);
//   }
// };

//end of unpacking tuple solution

class FuncWrap {
public:

    class FuncBase {
    public:
        virtual void Call() = 0;
        virtual ~FuncBase(){}
    };
  
    std::unique_ptr< FuncBase > _func;

    template < typename F , typename ... Args >
    class FuncImpl : public FuncBase {
    public:
        F _f;   
        std::tuple<Args... > _params;

        void delayed_dispatch()
            {
                callFunc( typename gens<sizeof...(Args)>::type());
            }

        template< int ...S >
        void callFunc( seq<S...> )
            {
                _f(std::get<S>(_params) ...);
            }

        FuncImpl( F && f, Args && ... args ) : _f( std::move( f ) ), _params( std::make_tuple< Args... > ( std::forward<Args>(args)... ) )
            {
            }
        // void Call() { std::cout<<"number of arguments: "<< std::tuple_size<decltype(_params)>::value << std::endl;}
        void Call() { delayed_dispatch(); }

    };

                template < typename F , typename ... Args >
                FuncWrap( F && f , Args && ... params ) : _func( new FuncImpl< F, Args... >(std::move( f ), std::forward<Args>(params)... ) ){}
                FuncWrap() = default;
                FuncWrap( FuncWrap && other ): _func( std::move( other._func ) ){}
  void          operator() () { _func->Call(); }
  FuncWrap &    operator= ( FuncWrap && other ) {
                  _func = std::move( other._func );
		  return *this;
                }
                FuncWrap( const FuncWrap & ) = delete;
                FuncWrap( FuncWrap & ) = delete;
  FuncWrap &    operator= ( const FuncWrap & ) = delete;
};

#endif
