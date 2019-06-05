#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <cassert>
#include <chrono>

#include "queue_lockfree_total.hpp"
#include "stress_pool.hpp"

using namespace std;

TEST_CASE( "queue_lockfree_total stress", "[stress]" ) {
    queue_lockfree_total<int> p;
    unsigned int num_threads = 10000;
    stress_pool::stress_put_get_int( num_threads, p );
}
