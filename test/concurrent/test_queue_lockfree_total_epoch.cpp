#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>

#include "catch.hpp"
#include "queue_lockfree_total.hpp"

using namespace std;

TEST_CASE( "queue_lockfree_total", "[queue push pop]" ) { 
    
    SECTION( "push-pop" ) {
    
        queue_lockfree_total<int, trait_reclamation::epoch> queue;

        queue_lockfree_total<int, trait_reclamation::epoch>::thread_init();

        size_t count = queue.size();
        CHECK( 0 == count );
        int val = 5;
        queue.put(val);
        count = queue.size();
        CHECK( 1 == count );

        auto ret = queue.get();
        count = queue.size();
        CHECK( 0 == count );
        CHECK( ret );
        CHECK( 5 == *ret );
    
        queue_lockfree_total<int, trait_reclamation::epoch>::thread_deinit();
        
        reclam_epoch<queue_lockfree_total<int, trait_reclamation::epoch>::Node>::clear_epoch_list();
    }
    SECTION( "pop_empty" ) {
    
        queue_lockfree_total<int, trait_reclamation::epoch> queue;

        queue_lockfree_total<int, trait_reclamation::epoch>::thread_init();
        
        size_t count;
        auto ret = queue.get();
        count = queue.size();
        CHECK( 0 == count );
        CHECK( !ret );
    
        queue_lockfree_total<int, trait_reclamation::epoch>::thread_deinit();

        reclam_epoch<queue_lockfree_total<int, trait_reclamation::epoch>::Node>::clear_epoch_list();
    }
    SECTION( "multiple instances" ) {
    
        queue_lockfree_total<int, trait_reclamation::epoch> q1,q2;

        queue_lockfree_total<int, trait_reclamation::epoch>::thread_init();

        CHECK( 0 == q1.size() );
        CHECK( 0 == q2.size() );
    
        q1.put(5);
        q2.put(6);

        CHECK( 1 == q1.size() );
        CHECK( 1 == q2.size() );
      
        auto b1 = q1.get();
        CHECK( 0 == q1.size() );
        auto b2 = q2.get();

        CHECK( b1 );
        CHECK( 5 == *b1 );
        CHECK( b2 );
        CHECK( 6 == *b2 );

        CHECK( 0 == q1.size() );
        CHECK( 0 == q2.size() );
    
        queue_lockfree_total<int, trait_reclamation::epoch>::thread_deinit();

        reclam_epoch<queue_lockfree_total<int, trait_reclamation::epoch>::Node>::clear_epoch_list();
    }
}

TEST_CASE( "queue_lockfree_total_multithread_long_lived", "[queue multithread longlived]" ) { 
    
    queue_lockfree_total<int, trait_reclamation::epoch> queue;

    unsigned int num_threads = std::thread::hardware_concurrency()/2;
    
    size_t nums = num_threads * 3000000;
    
    vector<int> retrieved( nums, 0 );

    vector<thread> threads( num_threads );
    
    auto t0 = std::chrono::high_resolution_clock::now();

    std::mutex m;
    std::condition_variable cv;
    int count_sync = 0;

    int temp = 0;
    for( int i = 0; i < num_threads; ++i ){
        threads[i] = std::thread( [ &, i ](){
                int dummy = 0;
                queue_lockfree_total<int, trait_reclamation::epoch>::thread_init();
                {//need a sync for all participating threads after thread_init due to implementation limitation
                    std::unique_lock<std::mutex> lk(m);
                    ++count_sync;
                    cv.notify_all();
                    cv.wait(lk, [&](){return count_sync>=num_threads;});
                }

                int val = nums/num_threads*i;
                for( int j = 0; j < nums/num_threads; ++j ){

                    while( !queue.put( val + j ) ){}

                    while(true){
                        if(auto ret = queue.get()){
                            ++retrieved[*ret];
                            dummy += *ret;
                            break;
                        }
                    }
                }

                {//need a sync for all participating threads before thread_deinit due to implementation limitation
                    std::unique_lock<std::mutex> lk(m);
                    ++count_sync;
                    cv.notify_all();
                    cv.wait(lk, [&](){return count_sync>=num_threads * 2;});
                }
                queue_lockfree_total<int, trait_reclamation::epoch>::thread_deinit();

                temp += dummy;
            } );
    }

    for( auto & i : threads ){
        i.join();
    }
    
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = t1 - t0;
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
    std::cout << "duration: " << dur_ms.count() << " ms. rate: " <<  (double)nums/dur_ms.count()*1000.0 << " put-get/sec." << std::endl;

    reclam_epoch<queue_lockfree_total<int, trait_reclamation::epoch>::Node>::clear_epoch_list();
        
    size_t count = queue.size();

    CHECK( 0 == count );

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
    CHECK( 0 == k );  
}
