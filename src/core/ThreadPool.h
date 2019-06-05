#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <future>
#include <memory>
#include <type_traits>
#include <utility>
#include <tuple>
#include <functional>

#include "FuncWrap.hpp"
#include "FuncWrap2.hpp"

class ThreadPool {
public:
    virtual void AddTaskHook( FuncWrap & fw ){}; /// implementable hook executed in AddTask, eg: adding to a buffer
    template < typename FuncType, typename ... Args >
    std::future < typename std::result_of< FuncType( Args... ) >::type > AddTask( FuncType f, Args ... params)
    {
        typedef typename std::result_of< FuncType( Args... ) >::type result_type;
        std::packaged_task< result_type( Args... ) > task( std::move( f ) );
        std::future< result_type > res( task.get_future() );
    
        FuncWrap fw( std::move(task), std::forward<Args>(params)... );
        AddTaskHook( fw );    /// call implemented function
        return res;
    }
    virtual bool GetTask( FuncWrap2 & fw ){ return false; } /// implementable interface
};

class ThreadPool2 {
public:
    virtual void AddTaskHook( FuncWrap2 & fw ){}; /// implementable hook executed in AddTask, eg: adding to a buffer
    template < typename FuncType, typename ... Args >
	void AddTask( FuncType f, Args ... params)
    {
	FuncWrap2 fw( f, std::forward<Args>(params)... );
        AddTaskHook( fw );    /// call implemented function
    }
    virtual bool GetTask( FuncWrap2 & fw ){ return false; } /// implementable interface
};

#endif
