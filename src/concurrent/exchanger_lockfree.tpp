#include <chrono>
#include <cassert>
#include <iostream>

template< class T >
exchanger_lockfree<T>::exchanger_lockfree(){
    _status.store(exchanger_status::EMPTY);
}
template< class T >
exchanger_lockfree<T>::~exchanger_lockfree(){
}

bool is_timeout(std::chrono::steady_clock::time_point start, long timeout_us ){
    std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
    auto diff = time_now - start;
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
    return duration > timeout_us;
}

template< class T >
bool exchanger_lockfree<T>::exchange( T & item, long timeout_us ){
    // T original = item;
    //try exchange with another thread via the exchanger node with specified timeout duration
    std::chrono::steady_clock::time_point time_enter = std::chrono::steady_clock::now();
    while(true){
        //test for timeout constraint
        if(is_timeout(time_enter, timeout_us)){
            //item = original;
            return false;
        }

        switch( _status.load( std::memory_order_relaxed ) ){
        case exchanger_status::EMPTY:
            {
                exchanger_status expected_empty = exchanger_status::EMPTY;
                if( _status.compare_exchange_strong( expected_empty, exchanger_status::EMPTY_2 ) ){
                    //at this point, current active thread is depositing value to be exchanged with 2nd arriving thread
                    _val = std::move(item);
                    
                    exchanger_status empty_2 = exchanger_status::EMPTY_2;

                    if(_status.compare_exchange_strong( empty_2, exchanger_status::WAITING)){
                    
                        //wait for 2nd thread to arrive
                        while(true){

                            if(is_timeout(time_enter, timeout_us)){
                                break;
                            }
                        
                            //wait for partner thread to exchange
                            exchanger_status expected_exchanging_2 = exchanger_status::EXCHANGING_2;
                            if( _status.compare_exchange_strong( expected_exchanging_2, exchanger_status::EXCHANGING_3 ) ){
                                //active thread received deposit from 2nd thread
                                item = std::move(_val);
                                exchanger_status expected_exchanging_3 = exchanger_status::EXCHANGING_3;
                                if(_status.compare_exchange_strong( expected_exchanging_3, exchanger_status::COMPLETE)){}
                            }
                        
                            exchanger_status complete2_status = exchanger_status::COMPLETE_2;
                            if(_status.compare_exchange_strong( complete2_status, exchanger_status::EMPTY)){
                                return true;
                            }
                        }

                        exchanger_status complete2_status = exchanger_status::COMPLETE_2;
                        if(_status.compare_exchange_strong( complete2_status, exchanger_status::EMPTY)){
                            return true;
                        }else{
                            exchanger_status s_empty2 = exchanger_status::EMPTY_2;
                            exchanger_status s_waiting = exchanger_status::WAITING;
                            exchanger_status s_exchanging = exchanger_status::EXCHANGING;
                            exchanger_status s_exchanging2 = exchanger_status::EXCHANGING_2;
                            exchanger_status s_exchanging3 = exchanger_status::EXCHANGING_3;
                            exchanger_status s_complete = exchanger_status::COMPLETE;
                            if(_status.compare_exchange_strong( s_empty2, exchanger_status::EMPTY)){
                                return false;
                            }
                            else if(_status.compare_exchange_strong( s_waiting, exchanger_status::EMPTY)){
                                return false;
                            }
                            else if(_status.compare_exchange_strong( s_exchanging, exchanger_status::EMPTY)){
                                return false;
                            }
                            else if(_status.compare_exchange_strong( s_exchanging2, exchanger_status::EMPTY)){
                                return false;
                            }
                            else if(_status.compare_exchange_strong( s_exchanging3, exchanger_status::EMPTY)){
                                return false;
                            }
                            else if(_status.compare_exchange_strong( s_complete, exchanger_status::EMPTY)){
                                return false;
                            }else{
                                return true;
                            }
                        }
                    }
                }
            }
            break;
        case exchanger_status::WAITING:
            {
                exchanger_status expected = exchanger_status::WAITING;
                if( _status.compare_exchange_strong( expected, exchanger_status::EXCHANGING ) ){
                    //at this point, this is the 2nd thread arriving, exchanging with an active thread
                    T val_exchanged(std::move(_val));
                    _val = std::move(item);
                    item = std::move(val_exchanged);

                    exchanger_status expected_exchanging = exchanger_status::EXCHANGING;
                    if(_status.compare_exchange_strong( expected_exchanging, exchanger_status::EXCHANGING_2)){
                        //signal exchange partner that object has been exchanged

                        while(true){

                            if( exchanger_status::EMPTY == _status.load()){
                                break;
                            }

                            if(is_timeout(time_enter, timeout_us)){
                                break;
                            }
                            
                            exchanger_status complete_status = exchanger_status::COMPLETE;
                            if(_status.compare_exchange_strong( complete_status, exchanger_status::COMPLETE_2)){
                                return true;
                            }
                            
                            std::this_thread::yield();
                        }
                    }
                    
                    return false;
                }
            }
            break;
        default:
            {
                //retry
                std::this_thread::yield();
            }
            break;
        }
    }
    return false;
}
