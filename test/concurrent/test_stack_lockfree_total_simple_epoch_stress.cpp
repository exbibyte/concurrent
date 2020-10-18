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

TEST_CASE( "stack_lockfree_total_simple_epoch stress", "[stress]" ) {
    using container_type = stack_lockfree_total_simple<int, trait_reclamation::epoch>;
    container_type p;
    unsigned int num_threads = std::thread::hardware_concurrency()/2;
    bool force_push_get = true;
    stress_pool::stress_put_get_int<container_type, container_type::mem_reclam>( num_threads, p, force_push_get );
    container_type::mem_reclam::clear_epoch_list();
    
}
