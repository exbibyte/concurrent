#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>

#include "catch.hpp"
#include "lock_recursive_block.hpp"

using namespace std;

void increase( int & val, lock_recursive_block & l, int recurse_lock ){
    for( int i = 0; i < recurse_lock; ++i ){
	l.lock();
	++val;
    }
    for( int i = 0; i < recurse_lock; ++i ){
	l.unlock();
    }
}

TEST_CASE( "queue_lockfree_total", "[queue]" ) {
    int increment = 0;
    lock_recursive_block lock;
    int n = 300;
    vector<std::thread> t(n);

    int recurse_lock = 20;

    auto t0 = std::chrono::high_resolution_clock::now();
    
    for( int i = 0; i < n; ++i ){
	t[i] = std::thread( increase, std::ref(increment), std::ref(lock), recurse_lock );
    }

    for( int i = 0; i < n; ++i ){
	t[i].join();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = t1 - t0;
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
		    
    CHECK( increment == n * recurse_lock );

    std::cout << "duration: " << dur_ms.count() << " ms. rate: " <<  (double)recurse_lock*n/dur_ms.count()*1000.0 << " lock-unlock/sec." << std::endl;
}
