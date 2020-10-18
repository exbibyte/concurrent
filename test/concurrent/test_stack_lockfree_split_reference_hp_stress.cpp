#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>

#include "catch.hpp"
#include "stack_lockfree_split_reference.hpp"
#include "stress_pool.hpp"

TEST_CASE( "stack_lockfree_split_reference stress", "[stress]" ) { 

    using container_type = stack_lockfree_split_reference<int, trait_reclamation::hp>;
    container_type p;
    unsigned int num_threads = std::thread::hardware_concurrency()/2;
    bool force_put_get = true;
    stress_pool::stress_put_get_int<container_type, container_type::mem_reclam>( num_threads, p, force_put_get );
}
