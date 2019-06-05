#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>

#include "catch.hpp"
#include "queue_lockfree_total.hpp"

using namespace std;

TEST_CASE( "queue_lockfree_total", "[queue]" ) { 

    SECTION( "push" ) {

        queue_lockfree_total<int> queue;
            
        size_t count = queue.size();
        CHECK( 0 == count );
        int val = 5;
        queue.put(val);
        count = queue.size();
        CHECK( 1 == count );

        SECTION( "pop" ) {
            int retrieve;
            bool bRet = queue.get( retrieve );
            count = queue.size();
            CHECK( 0 == count );
            CHECK( true == bRet );
            CHECK( 5 == retrieve );
        }
    }    

    SECTION( "pop empty" ) {

        queue_lockfree_total<int> queue;
        
        int retrieve;
        size_t count;
        bool bRet = queue.get( retrieve );
        count = queue.size();
        CHECK( 0 == count );
        CHECK( false == bRet );
    }
    
    SECTION( "multi-thread push-pop" ) {

        queue_lockfree_total<int> queue;

        int count_loop = 20;
        while( --count_loop >=0 ){
            size_t count;
            unsigned int num_threads = 100;
            vector<thread> threads( num_threads );
            for( int i = 0; i < num_threads; ++i ){
                threads[i] = std::thread( [ &, i ](){
                        int val = i;
                        queue.put( val );
                    } );
            }
            count = queue.size();
            cout << "queue size after push threads started: " << count << endl;

            vector<thread> threads2( num_threads );
            set<int> vals_retrieve;
            for( int i = 0; i < num_threads * 0.1; ++i ){
                threads2[i] = std::thread( [&](){
                        int pop_val;
                        bool bRet = queue.get( pop_val );
                        if( bRet ){
                            // std::cout << pop_val << std::endl;
                        }
                    } );
            }
            for( int i = 0; i < num_threads * 0.1; ++i ){
                threads2[i].join();
            }
            for( auto & i : threads ){
                i.join();
            }
            count = queue.size();
            CHECK( (num_threads * 0.9 ) == count );

            for( int i = 0; i < num_threads * 0.9; ++i ){
                threads2[i] = std::thread( [&](){
                        int pop_val;
                        bool bRet = queue.get( pop_val );
                        if( bRet ){
                            // std::cout << pop_val << std::endl;
                        }
                    } );
            }
            for( int i = 0; i < num_threads * 0.9; ++i ){
                threads2[i].join();
            }
            count = queue.size();
            CHECK( 0 == count );
        }
    }
}
