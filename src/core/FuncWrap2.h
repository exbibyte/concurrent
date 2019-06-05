//Deprecated. See Funwrap3.
#ifndef FUNCWRAP2_H
#define FUNCWRAP2_H

#include <thread>
#include <functional>
#include <type_traits>

class FuncBase {
public:
    FuncBase(){}
    virtual void Call(){}
    virtual ~FuncBase(){}
    virtual void * GetResult(){ return 0; }
};

template< class F, class... Args >
class FuncImpl : public FuncBase {
public:
    using ResultType = typename std::result_of<F(Args...) >::type;
    std::function< ResultType() > _func;
    ResultType _result;
    FuncImpl( F f, Args... args ){
	_func = std::function<ResultType()>([=]()->ResultType {
		return f( args... );
	    });
    }
    ResultType Dispatch(){
	return _func();
    }
    void Call() override {
        _result = Dispatch();
    }
    void * GetResult() override {
	return (void *) &_result;
    }
};

class FuncWrap2 {
public:
    mutable FuncBase * _funcbase;
    template< class F, class... Args >
    FuncWrap2( F f, Args... args ) : _funcbase(new FuncImpl< F, Args...>( f, args... )) {
    }
    ~FuncWrap2(){
	if( _funcbase ){
	    delete _funcbase;
	}
    }
    FuncWrap2(){
	_funcbase = nullptr;
    }
    void operator()(){
	_funcbase->Call();
    }
    FuncWrap2( const FuncWrap2 & other ){
	_funcbase = other._funcbase;
	other._funcbase = nullptr;
    }
    FuncWrap2 & operator= ( FuncWrap2 & other ) {
    	_funcbase = other._funcbase;
    	other._funcbase = nullptr;
    	return *this;
    }
};

#endif
