#ifndef STRESS_POOL_HPP
#define STRESS_POOL_HPP

#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <condition_variable>

#include "threadwrap.hpp"

class stress_pool {
public:
    template< typename Pool, typename ... mem_manager >
    static void stress_put_get_int( unsigned int num_threads, Pool & pool, bool force_push_get = false ){
        std::vector<threadwrap> threads2( num_threads );
        std::vector<threadwrap> threads( num_threads );    
        int count_loop = 1;
        int num_data_per_thread = 1000000;
        while( --count_loop >=0 ){
            size_t count;
            auto t0 = std::chrono::steady_clock::now();
            int sync = 0;
            int sync2 = 0;
            std::mutex m;
            std::condition_variable cv;
            for( int i = 0; i < num_threads; ++i ){
                threads[i] = threadwrap( [ &, i ](){
                        // Pool::thread_init();
                        {
                            std::scoped_lock<std::mutex> l(m);
                            ++sync;
                        }
                        cv.notify_all();
                        {
                            std::unique_lock<std::mutex> l(m);
                            cv.wait(l, [&](){return sync>=2*num_threads;});
                        }
                        
                        for(int j=0; j<num_data_per_thread; ++j){
                            int val = j;
                            if(force_push_get){
                                while(true){
                                    if(pool.put( val )){
                                        break;
                                    }
                                }
                            }else{
                                pool.put( val );
                            }
                        }
                        {
                            std::scoped_lock<std::mutex> l(m);
                            ++sync2;
                        }
                        cv.notify_all();
                        {
                            std::unique_lock<std::mutex> l(m);
                            cv.wait(l, [&](){return sync2>=2*num_threads;});
                        }
                        // Pool::thread_deinit();
                    }, identity<mem_manager>()... );
            }
            for( int i = 0; i < num_threads; ++i ){
                threads2[i] = threadwrap( [&](){
                        // Pool::thread_init();
                        {
                            std::scoped_lock<std::mutex> l(m);
                            ++sync;
                        }
                        cv.notify_all();
                        {
                            std::unique_lock<std::mutex> l(m);
                            cv.wait(l, [&](){return sync>=2*num_threads;});
                        }
                        int retries = 0;
                        for(int j=0; j<num_data_per_thread; ++j){
                            if(force_push_get){
                                while(true){
                                    if( auto pop_val = pool.get(); pop_val ){
                                        retries = 0;
                                        break;
                                    }
                                }
                            }else{
                                auto pop_val = pool.get();
                            }
                        }
                        {
                            std::scoped_lock<std::mutex> l(m);
                            ++sync2;
                        }
                        cv.notify_all();
                        {
                            std::unique_lock<std::mutex> l(m);
                            cv.wait(l, [&](){return sync2>=2*num_threads;});
                        }
                        // Pool::thread_deinit();
                    }, identity<mem_manager>()... );
            }
            for( auto & i : threads ){
                i.join();
            }            
            for( auto & i : threads2 ){
                i.join();
            }
            
            count = pool.size();
            if(force_push_get){
                CHECK( 0 ==  count );
            }
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> dur = t1 - t0;
            auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
            std::cout << "duration: " << dur_ms.count() << " ms. rate: " <<  (double)num_threads * num_data_per_thread/dur_ms.count()*1000.0 << " put-get/sec." << std::endl;
        }
    }
};

#endif
