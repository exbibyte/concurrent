#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>

#include "catch.hpp"
#include "stack_lockfree_total_simple.hpp"

using namespace std;

TEST_CASE( "stack_lockfree_total_simple", "[stack]" ) { 

    {
        stack_lockfree_total_simple<int, trait_reclamation::epoch> stack;
        stack_lockfree_total_simple<int, trait_reclamation::epoch>::thread_init();
        SECTION( "put" ) {
            size_t count = stack.size();
            CHECK( 0 == count );
            int num = 5;
            stack.put(num);
            count = stack.size();
            CHECK( 1 == count );

            SECTION( "get" ) {
                auto ret = stack.get();
                count = stack.size();
                CHECK( 0 == count );
                CHECK( ret );
                CHECK( 5 == *ret );
            }
        }    

        SECTION( "get empty" ) {
            size_t count;
            auto ret = stack.get();
            count = stack.size();
            CHECK( 0 == count );
            CHECK( !ret );
        }
        stack_lockfree_total_simple<int, trait_reclamation::epoch>::thread_deinit();
    }
    reclaim_epoch<stack_lockfree_total_simple<int, trait_reclamation::epoch>::Node>::clear_epoch_list();

    // {
    //     stack_lockfree_total_simple<int, trait_reclamation::epoch> stack;
        
    //     SECTION( "multi-thread put" ) {
    //         size_t count;
    //         unsigned int num_threads = 10;
    //         vector<thread> threads( num_threads );
    //         std::mutex m;
    //         std::condition_variable cv;
    //         int sync = 0;
    //         for( int i = 0; i < num_threads; ++i ){
    //             threads[i] = std::thread( [ &, i ](){
    //                     stack_lockfree_total_simple<int, trait_reclamation::epoch>::thread_init();
    //                     {
    //                         std::scoped_lock<std::mutex> l(m);
    //                         sync++;
    //                     }
    //                     cv.notify_all();
    //                     {
    //                         std::unique_lock<std::mutex> l(m);
    //                         cv.wait(l, [&](){ return sync >= num_threads; });
    //                     }
                    
    //                     int num = i;
    //                     stack.put( num );

    //                     {
    //                         std::scoped_lock<std::mutex> l(m);
    //                         sync++;
    //                     }
    //                     cv.notify_all();
    //                     {
    //                         std::unique_lock<std::mutex> l(m);
    //                         cv.wait(l, [&](){ return sync >= 2*num_threads; });
    //                     }
    //                     stack_lockfree_total_simple<int, trait_reclamation::epoch>::thread_deinit();
    //                 } );
    //         }
    //         for( auto & i : threads ){
    //             i.join();
    //         }
    //         count = stack.size();
    //         CHECK( num_threads == count );

    //         sync = 0;

    //         SECTION( "multi-thread get" ) {
    //             vector<int> vals_retrieve(num_threads, 0);
    //             for( auto & i : threads ){
    //                 i = std::thread( [&](){
    //                         stack_lockfree_total_simple<int, trait_reclamation::epoch>::thread_init();
    //                         {
    //                             std::scoped_lock<std::mutex> l(m);
    //                             sync++;
    //                         }
    //                         cv.notify_all();
    //                         {
    //                             std::unique_lock<std::mutex> l(m);
    //                             cv.wait(l, [&](){ return sync >= num_threads; });
    //                         }

    //                         while(true){//force retrieval
    //                             if( auto ret = stack.get() ){
    //                                 vals_retrieve[*ret]++;
    //                                 break;
    //                             }    
    //                         }

    //                         {
    //                             std::scoped_lock<std::mutex> l(m);
    //                             sync++;
    //                         }
    //                         cv.notify_all();
    //                         {
    //                             std::unique_lock<std::mutex> l(m);
    //                             cv.wait(l, [&](){ return sync >= 2*num_threads; });
    //                         }
    //                         stack_lockfree_total_simple<int, trait_reclamation::epoch>::thread_deinit();
    //                     } );
    //             }
    //             for( auto & i : threads ){
    //                 i.join();
    //             }
    //             count = stack.size();
    //             CHECK( 0 == count );
    //             for( int i = 0; i < num_threads; ++i ){
    //                 CHECK(vals_retrieve[i]==1);
    //             }
    //         }
    //     }
    //     reclaim_epoch<stack_lockfree_total_simple<int, trait_reclamation::epoch>::Node>::clear_epoch_list();
    // }
}
