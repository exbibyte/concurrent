#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include "catch.hpp"
#include "elimination_array.hpp"
#include <functional>

#include <chrono>
#include <thread>

using namespace std;

TEST_CASE( "elimination_array size 1", "[elimination_array]" ) {
    constexpr size_t elim_size = 1;
    elimination_array<int, elim_size> elim;
    long timeout_us = 10000;
    elim.set_timeout( timeout_us );

    function<void(bool*,int*)> f = [&](bool* b, int * a){
	*b = elim.visit( *a, elim_size-1 );
    };

    int n= 100;
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

    std::cout << "threads joined." << std::endl;

    vector<int> expected_vals(n);
    count = 0;
    for( auto & i : expected_vals ){
    	i = count;
    	++count;
    }
    //check each value has changed
    int count_eliminated = 0;
    for( size_t i = 0; i < n; ++i ){
	if( rets[i] ){
	    ++count_eliminated;
	}
    }

    std::cout << "num thread: " << n << ", size elimnation array: " << elim_size << ", percent eliminated: " << (double)count_eliminated / n * 100.0 << std::endl;

    //check appearance of each unique value
    sort(vals.begin(), vals.end());
    bool vals_unique = expected_vals == vals;
    CHECK( vals_unique );

    delete [] rets;
}

TEST_CASE( "elimination_array visit range exceed size", "[elimination_array]" ) {
    constexpr size_t elim_size = 1;
    elimination_array<int, elim_size> elim;
    long timeout_us = 10000;
    elim.set_timeout( timeout_us );

    function<void(bool*,int*)> f = [&](bool* b, int * a){
	*b = elim.visit( *a, 5 );
    };

    int n= 10;
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

    std::cout << "threads joined." << std::endl;

    vector<int> expected_vals(n);
    count = 0;
    for( auto & i : expected_vals ){
    	i = count;
    	++count;
    }
    //check each value has changed
    int count_eliminated = 0;
    for( size_t i = 0; i < n; ++i ){
	if( rets[i] ){
	    ++count_eliminated;
	}
    }

    std::cout << "num thread: " << n << ", size elimnation array: " << elim_size << ", percent eliminated: " << (double)count_eliminated / n * 100.0 << std::endl;

    //check appearance of each unique value
    sort(vals.begin(), vals.end());
    bool vals_unique = expected_vals == vals;
    CHECK( vals_unique );

    delete [] rets;
}
