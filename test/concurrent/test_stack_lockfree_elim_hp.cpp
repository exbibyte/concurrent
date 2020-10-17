#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>

#include "catch.hpp"
#include "stack_lockfree_elim.hpp"
#include "threadwrap.hpp"

using namespace std;

using container_type = stack_lockfree_elim<int, trait_reclamation::hp>;

TEST_CASE( "stack_lockfree_elim_hp", "[stack]" ) { 

    container_type stack;

    auto f = [&](){
        
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

    };
    threadwrap::this_thread_run<container_type::mem_reclam>(f);

    SECTION( "multi-thread put" ) {
        size_t count;
        unsigned int num_threads = 10;
        vector<threadwrap> threads( num_threads );
        for( int i = 0; i < num_threads; ++i ){
            threads[i] = threadwrap( [ &stack, i ](){
                    int num = i;
                    while(!stack.put( num )){}
                }, identity<container_type::mem_reclam>() );
        }
        for( auto & i : threads ){
            i.join();
        }
        count = stack.size();
        CHECK( num_threads == count );

        SECTION( "multi-thread get" ) {
            vector<int> vals_retrieve(num_threads, 0);
            for( auto & i : threads ){
                i = threadwrap( [&](){
                        while(true){//force retrieval
                            if( auto ret = stack.get() ){
                                vals_retrieve[*ret]++;
                                break;
                            }    
                        }
                    }, identity<container_type::mem_reclam>() );
            }
            for( auto & i : threads ){
                i.join();
            }
            count = stack.size();
            CHECK( 0 == count );
            for( int i = 0; i < num_threads; ++i ){
                CHECK(vals_retrieve[i]==1);
            }
        }
    }
}
