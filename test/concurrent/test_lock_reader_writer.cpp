#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>

#include "catch.hpp"
#include "lock_reader_writer_sync.hpp"

using namespace std;

void increase( int & val, lock_reader_writer_sync & l, int recurse_lock ){
    for( int i = 0; i < recurse_lock; ++i ){
	l.lock( lock_access_type::WRITE );
	++val;
	l.unlock( lock_access_type::WRITE );
    }
}

void readval( int & val, lock_reader_writer_sync & l, int recurse_lock, vector<int> & r, int index ){
    for( int i = 0; i < recurse_lock; ++i ){
	l.lock( lock_access_type::READ );
	r[index] = val;
	l.unlock( lock_access_type::READ );
    }
}

TEST_CASE( "lock_reader_writer_sync", "[lock]" ) {
    int increment = 0;
    lock_reader_writer_sync lock;
    int n = 1000;
    vector<std::thread> t(n);
    int recurse_lock = 20;
    vector<int> reads(n,-1);
    auto t0 = std::chrono::high_resolution_clock::now();
    for( int i = 0; i < n; ++i ){
	if( i % 2 == 0 )
	    t[i] = std::thread( increase, std::ref(increment), std::ref(lock), recurse_lock );
	else
	    t[i] = std::thread( readval, std::ref(increment), std::ref(lock), recurse_lock, std::ref(reads), i );
    }
    for( int i = 0; i < n; ++i ){
	t[i].join();
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = t1 - t0;
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
    CHECK( increment == n * recurse_lock / 2 );
    int read_check = 0;
    for( auto i : reads ){
	if( i >= 0 )
	    ++read_check;
    }
    CHECK( read_check == n / 2 );
    CHECK( lock.count() == 0 );
    std::cout << "duration: " << dur_ms.count() << " ms. rate: " <<  (double)recurse_lock*n/dur_ms.count()*1000.0 << " lock-unlock/sec." << std::endl;
}
