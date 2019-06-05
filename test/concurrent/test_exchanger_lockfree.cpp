#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include "catch.hpp"
#include "exchanger_lockfree.hpp"
#include <functional>

#include <chrono>
#include <thread>

using namespace std;

TEST_CASE( "exchanger_lockfree", "[exchanger]" ) { 
    exchanger_lockfree<int> ex;
    long timeout_us = 100000;

    function<void(bool*,int*)> f = [&](bool* b, int * a){
	*b = ex.exchange( *a, timeout_us );
    };

    int n= 200;
    vector<int> vals(n);
    int count = 0;
    for( auto & i : vals ){
    	i = count;
    	++count;
    }
    bool * rets = new bool[n];
    vector<thread> ts(n);

    auto start = std::chrono::high_resolution_clock::now();
    
    for(size_t i = 0; i < n; ++i ){
	bool * b = &rets[i];
        int * a = &vals[i];
    	ts[i] = std::thread( f, b, a );
    }

    for(size_t i = 0; i < n; ++i ){
    	ts[i].join();
    }

    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now-start).count();    
    std::cout << "threads joined." << std::endl;
    std::cout << "duration(ms): " << duration << std::endl;

    vector<int> expected_vals(n);
    count = 0;
    for( auto & i : expected_vals ){
    	i = count;
    	++count;
    }
    //check each value has changed
    for( size_t i = 0; i < n; ++i ){
    	CHECK( rets[i] == true );
    	CHECK( vals[i] != expected_vals[i] );
    }
    //check appearance of each unique value
    sort(vals.begin(), vals.end());
    bool vals_unique = expected_vals == vals;
    CHECK( vals_unique );

    delete [] rets;
}
TEST_CASE( "exchanger_lockfree timeout", "[exchanger]" ) { 
    exchanger_lockfree<int> ex;
    long timeout_us = 3'000'000;

    function<void(bool*,int*)> f = [&](bool* b, int * a){
	*b = ex.exchange( *a, timeout_us );
    };

    int n= 1;
    vector<int> vals(n);
    int count = 0;
    for( auto & i : vals ){
    	i = count;
    	++count;
    }
    bool * rets = new bool[n];
    vector<thread> ts(n);
    for(size_t i = 0; i < n; ++i ){
	bool * b = &rets[i];
        int * a = &vals[i];
    	ts[i] = std::thread( f, b, a );
    }

    for(size_t i = 0; i < n; ++i ){
    	ts[i].join();
    }
    CHECK( false == rets[0] );
    auto stat = ex._status.load();
    CHECK( exchanger_status::ABORT == stat );
    std::cout << "threads joined." << std::endl;
}
