#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>
#include <thread>

#include "queue_lockfree_sync.hpp"
#include "stress_pool.hpp"

using namespace std;

TEST_CASE( "queue_lockfree_sync stress", "[stress]" ) { 
    queue_lockfree_sync<int> p;
    unsigned int num_threads = 200;
    stress_pool::stress_put_get_int( num_threads, p );
}
