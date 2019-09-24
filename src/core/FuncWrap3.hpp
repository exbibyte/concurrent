//Deprecated. See Funwrap3.
#ifndef FUNCWRAP3_H
#define FUNCWRAP3_H

#include <thread>
#include <functional>
#include <type_traits>

enum class FunCallType {
    SYNC,
    ASYNC,	
};

class Funwrap3 {
public:
    Funwrap3() : _fun(nullptr), _calltype(FunCallType::ASYNC) {}
    template< class F, class... Args >
    void set( FunCallType calltype, F f, Args... args ){
	_calltype = calltype;
	using ResultType = typename std::result_of<F(Args...) >::type;
	//std::function< ResultType() > _fun;
	// ResultType _result;
	_fun = std::function<void()>([=]()->ResultType {
		ResultType result = f( args... ); //ignore result
		return;
	    });
    }
    void apply(){
	if( nullptr != _fun ){
	    _fun();
	}
	_fun = nullptr;
    }
private:
    std::function<void()> _fun;
    FunCallType _calltype;
    
};

#endif
