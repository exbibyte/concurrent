#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>

#include "catch.hpp"
#include "stack_lockfree_total_simple.hpp"

using namespace std;

using container_type = stack_lockfree_total_simple<int, trait_reclamation::epoch>;

TEST_CASE( "stack_lockfree_total_simple_epoch", "[stack]" ) { 

    {
        container_type stack;
        container_type::mem_reclam::thread_init();
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
        container_type::mem_reclam::thread_deinit();
    }
}
