#ifndef STRESS_POOL_HPP
#define STRESS_POOL_HPP

#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

class stress_pool {
public:
    template< typename Pool >
    static void stress_put_get_int( unsigned int num_threads, Pool & pool ){
        std::vector<std::thread> threads2( num_threads );
        std::vector<std::thread> threads( num_threads );    
        int count_loop = 5;
        while( --count_loop >=0 ){
            size_t count;
            auto t0 = std::chrono::high_resolution_clock::now();
            for( int i = 0; i < num_threads; ++i ){
                threads[i] = std::thread( [ &, i ](){
                        int val = i;
                        pool.put( val );
                    } );
            }
            count = pool.size();
            CHECK( 0 <= count );
            for( int i = 0; i < num_threads; ++i ){
                threads2[i] = std::thread( [&](){
                        auto pop_val = pool.get();
                    } );
            }
            for( auto & i : threads ){
                i.join();
            }
            for( auto & i : threads2 ){
                i.join();
            }
            count = pool.size();
            CHECK( 0 ==  count );
            auto t1 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> dur = t1 - t0;
            auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
            std::cout << "duration: " << dur_ms.count() << " ms. rate: " <<  (double)num_threads/dur_ms.count()*1000.0 << " put-get/sec." << std::endl;
        }
    }
};

#endif
