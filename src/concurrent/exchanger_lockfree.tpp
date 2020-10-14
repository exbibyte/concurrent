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
template< class T >
bool exchanger_lockfree<T>::exchange( T & item, long timeout_us ){
    T original = item;
    //try exchange with another thread via the exchanger node with specified timeout duration
    std::chrono::steady_clock::time_point time_enter = std::chrono::steady_clock::now();
    while(true){
        //test for timeout constraint
        std::chrono::steady_clock::time_point time_now = std::chrono::steady_clock::now();
        auto diff = time_now - time_enter;
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
        if( duration > timeout_us ){
#ifdef DEBUG_VERBOSE
            std::cout << "entering: timeout segment 1." << std::endl;
#endif
            item = original;
            return false;
        }

        switch( _status.load( std::memory_order_acquire ) ){
        case exchanger_status::EMPTY:
            {
                exchanger_status expected_empty = exchanger_status::EMPTY;
                if( _status.compare_exchange_weak( expected_empty, exchanger_status::EMPTY_2, std::memory_order_acq_rel ) ){
                    //at this point, current active thread is depositing value to be exchanged with 2nd arriving thread
                    exchanger_status waiting = exchanger_status::WAITING;
                    _val = item;
                    _status.store( waiting, std::memory_order_release );
                    //wait for 2nd thread to arrive
                    while(true){
                        time_now = std::chrono::steady_clock::now();
                        diff = time_now - time_enter;
                        duration = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
                        if( duration > timeout_us ){
                            break;
                        }
                        //wait for partner thread to exchange
                        exchanger_status expected_exchanging_2 = exchanger_status::EXCHANGING_2;
                        if( _status.compare_exchange_weak( expected_exchanging_2, exchanger_status::EXCHANGING_3, std::memory_order_acq_rel ) ){
                            //active thread received deposit from 2nd thread
                            exchanger_status complete = exchanger_status::COMPLETE;
                            item = _val;
                            _status.store( complete, std::memory_order_release );
#ifdef DEBUG_VERBOSE
                            std::cout << "entering: exchanged with partner segment 1." << std::endl;
#endif
                            return true;
                        }
                    }
                    exchanger_status expected_exchanging_2 = exchanger_status::EXCHANGING_2;
                    if( _status.compare_exchange_weak( expected_exchanging_2, exchanger_status::EXCHANGING_3, std::memory_order_acq_rel ) ){
                        //active thread received deposit from 2nd thread
                        exchanger_status complete = exchanger_status::COMPLETE;
                        item = _val;
                        _status.store( complete, std::memory_order_release );
#ifdef DEBUG_VERBOSE
                        std::cout << "entering: exchanged with partner segment 1." << std::endl;
#endif
                        return true;
                    }else{
                        //active thread times out and gives up resource
#ifdef DEBUG_VERBOSE
                        std::cout << "entering: timeout segment 2." << std::endl;
#endif
                        exchanger_status abort = exchanger_status::ABORT;
                        _status.store( abort, std::memory_order_release );
                        item = original;
                        return false;
                    }
                }
            }
            break;
        case exchanger_status::WAITING:
            {
                exchanger_status expected = exchanger_status::WAITING;
                if( _status.compare_exchange_weak( expected, exchanger_status::EXCHANGING, std::memory_order_acq_rel ) ){
                    //at this point, this is the 2nd thread arriving, exchanging with an active thread
                    exchanger_status reset = exchanger_status::EMPTY;
                    exchanger_status exchanging_2 = exchanger_status::EXCHANGING_2;
                    T val_exchanged = _val;
                    _val = item;
                    item = val_exchanged;
                    _status.store( exchanging_2, std::memory_order_release ); //signal exchange partner that object has been exchanged
                    exchanger_status final_status = _status.load( std::memory_order_acquire );
                    while( true ){
                        if( exchanger_status::COMPLETE == final_status ){
                            _status.store( reset, std::memory_order_release );
#ifdef DEBUG_VERBOSE
                            std::cout << "entering: exchanged with partner segment 2." << std::endl;
#endif
                            return true;
                        }else if( exchanger_status::ABORT == final_status ){
                            _status.store( reset, std::memory_order_release );
                            item = original;
                            return false;
                        }
                        time_now = std::chrono::steady_clock::now();
                        diff = time_now - time_enter;
                        duration = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
                        if( duration > timeout_us ){
                            break;
                        }
                        final_status = _status.load( std::memory_order_acquire );
                    }
                    _status.store( reset, std::memory_order_release );
                    item = original;
                    return false;
                }
            }
            break;
        case exchanger_status::ABORT:
            {
                exchanger_status aborted = exchanger_status::ABORT;
                _status.compare_exchange_weak( aborted, exchanger_status::EMPTY, std::memory_order_acq_rel );
                //retry
            }
            break;
        default:
            //retry
            break;
        }
    }
    item = original;
    return false;
}
