#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>
#include <cassert>

#include "queue_lockfree_total.hpp"
#include "threadwrap.hpp"
#include "reclam_hazard.hpp"

using namespace std;

using container_type = queue_lockfree_total<int, trait_reclamation::hp>;

int main(){
    
    container_type queue;

    unsigned int num_threads = std::thread::hardware_concurrency()/2;
    
    size_t nums = num_threads * 3000000;
    
    vector<int> retrieved( nums, 0);

    vector<threadwrap> threads( num_threads );
    vector<threadwrap> threads2( num_threads );
    
    auto t0 = std::chrono::high_resolution_clock::now();

    for( int i = 0; i < num_threads; ++i ){
        threads[i] = threadwrap( [ &, i ](){
                int val = nums/num_threads*i;
                for( int j = 0; j < nums/num_threads; ++j ){
                    while( !queue.put( val + j ) ){} //force enqueue
                }
            }, identity<container_type::mem_reclam>() );
    }

    for( int i = 0; i < num_threads; ++i ){
        threads2[i] = threadwrap( [&](){
                for( int j = 0; j < nums/num_threads; ++j ){
                    while(true){
                        if(auto ret = queue.get()){
                            ++retrieved[*ret];
                            break;
                        }
                        std::this_thread::yield();
                    } 
                }
            }, identity<container_type::mem_reclam>() );
    }
  
    for( auto & i : threads ){
        i.join();
    }
    for( auto & i : threads2 ){
        i.join();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = t1 - t0;
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
    std::cout << "duration: " << dur_ms.count() << " ms. rate: " <<  (double)nums/dur_ms.count()*1000.0 << " put-get/sec." << std::endl;
    
    size_t count = queue.size();

    assert( 0 == count );

    int k = 0;
    int n = 0;
    //expect count of 1 for each retrieved number
    for(auto i: retrieved){
        if(i!=1){
            //oops, something is wrong, write out count
            ++k;
            std::cout << n << ": " << i << std::endl;
        }
        ++n;
    }
    assert( 0 == k );
    
    return 0;
}
