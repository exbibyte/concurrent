#ifndef STRESS_POOL_HPP
#define STRESS_POOL_HPP

#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <mutex>
#include <condition_variable>

class stress_pool {
public:
    template< typename Pool >
    static void stress_put_get_int( unsigned int num_threads, Pool & pool, bool force_push_get = false ){
        std::vector<std::thread> threads2( num_threads );
        std::vector<std::thread> threads( num_threads );    
        int count_loop = 5;
        int num_data_per_thread = 2000000;
        while( --count_loop >=0 ){
            size_t count;
            auto t0 = std::chrono::high_resolution_clock::now();
            for( int i = 0; i < num_threads; ++i ){
                threads[i] = std::thread( [ &, i ](){
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
                        Pool::thread_deinit();
                    } );
            }
            for( int i = 0; i < num_threads; ++i ){
                threads2[i] = std::thread( [&](){
                        int retries = 0;
                        for(int j=0; j<num_data_per_thread; ++j){
                            if(force_push_get){
                                while(true){
                                    if( auto pop_val = pool.get(); pop_val ){
                                        retries = 0;
                                        break;
                                    }else{
                                        std::this_thread::yield();
                                    }
                                }
                            }else{
                                auto pop_val = pool.get();
                            }
                        }
                        Pool::thread_deinit();
                    } );
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
            auto t1 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> dur = t1 - t0;
            auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
            std::cout << "duration: " << dur_ms.count() << " ms. rate: " <<  (double)num_threads * num_data_per_thread/dur_ms.count()*1000.0 << " put-get/sec." << std::endl;
        }
    }
};

#endif
