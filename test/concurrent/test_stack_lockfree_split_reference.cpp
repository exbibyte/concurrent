#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>

#include "catch.hpp"
#include "stack_lockfree_split_reference.hpp"

using namespace std;

TEST_CASE( "stack_lockfree_split_reference", "[stack split reference]" ) { 

    stack_lockfree_split_reference<int, trait_reclamation::not_applicable> stack;

    SECTION( "put" ) {
        size_t count = stack.size();
        CHECK( 0 == count );
        stack.put(5);
        stack.put(10);
        count = stack.size();
        CHECK( 2 == count );

        SECTION( "get" ) {
            auto ret = stack.get();
            count = stack.size();
            CHECK( 1 == count );
            CHECK( ret );
            CHECK( 10 == *ret );
        }
    }    

    SECTION( "get empty" ) {
        auto ret = stack.get();
        size_t count = stack.size();
        CHECK( 0 == count );
        CHECK( !ret );
    }

    SECTION( "multi-thread put" ) {
        size_t count;
        unsigned int num_threads = 10;
        vector<thread> threads( num_threads );
        for( int i = 0; i < num_threads; ++i ){
            threads[i] = std::thread( [ &stack, i ](){
                    stack.put( i );
                } );
        }
        for( auto & i : threads ){
            i.join();
        }
        count = stack.size();
        CHECK( num_threads == count );

        SECTION( "multi-thread get" ) {
            set<int> vals_retrieve;
            mutex mtx;
            for( auto & i : threads ){
                i = std::thread( [&](){
                        auto ret = stack.get();
                        mtx.lock();
                        if( ret ){
                            vals_retrieve.insert( *ret );
                        }
                        mtx.unlock();
                    } );
            }
            for( auto & i : threads ){
                i.join();
            }
            count = stack.size();
            CHECK( 0 == count );
            for( int i = 0; i < num_threads; ++i ){
                auto it = vals_retrieve.find(i);
                CHECK( vals_retrieve.end() != it );
                if( vals_retrieve.end() != it )
                    vals_retrieve.erase(it);
            }

            for( auto & i : threads ){
                i = std::thread( [&](){
                        auto ret = stack.get();
                        mtx.lock();
                        if( ret ){
                            vals_retrieve.insert( *ret );
                        }
                        mtx.unlock();
                    } );
            }
            for( auto & i : threads ){
                i.join();
            }
            CHECK( vals_retrieve.empty() );
        }
    }
}
