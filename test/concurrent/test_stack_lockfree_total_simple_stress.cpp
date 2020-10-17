#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>

#include "catch.hpp"
#include "stack_lockfree_total_simple.hpp"
#include "stress_pool.hpp"

using namespace std;

TEST_CASE( "stack_lockfree_total_simple stress", "[stress]" ) {
    // stack_lockfree_total_simple<int, trait_reclamation::not_applicable> p;
    // // unsigned int num_threads = 10000;
    // unsigned int num_threads = std::thread::hardware_concurrency();
    // stress_pool::stress_put_get_int( num_threads, p );
}
