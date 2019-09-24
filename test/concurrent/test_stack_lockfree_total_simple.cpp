#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>

#include "catch.hpp"
#include "stack_lockfree_total_simple.hpp"

using namespace std;

TEST_CASE( "stack_lockfree_total_simple", "[stack]" ) { 

    stack_lockfree_total_simple<int, trait_reclamation::not_applicable> stack;

    SECTION( "put" ) {
	size_t count = stack.size();
	CHECK( 0 == count );
	int num = 5;
	stack.put(num);
	count = stack.size();
	CHECK( 1 == count );

	SECTION( "get" ) {
	    int retrieve;
	    bool bRet = stack.get( retrieve );
	    count = stack.size();
	    CHECK( 0 == count );
	    CHECK( true == bRet );
	    CHECK( 5 == retrieve );
	}
    }    

    SECTION( "get empty" ) {
	int retrieve;
	size_t count;
	bool bRet = stack.get( retrieve );
	count = stack.size();
	CHECK( 0 == count );
	CHECK( false == bRet );
    }

    SECTION( "multi-thread put" ) {
	size_t count;
	unsigned int num_threads = 10;
	vector<thread> threads( num_threads );
	for( int i = 0; i < num_threads; ++i ){
	    threads[i] = std::thread( [ &stack, i ](){
		    int num = i;
		    stack.put( num );
		} );
	}
	for( auto & i : threads ){
	    i.join();
	}
	count = stack.size();
	CHECK( num_threads == count );

	SECTION( "multi-thread get" ) {
	    set<int> vals_retrieve;
	    mutex mtx;
	    for( auto & i : threads ){
		i = std::thread( [&](){
			int get_val;
			bool bRet = stack.get( get_val );
			mtx.lock();
			if( bRet ){
			    vals_retrieve.insert( get_val );
			}
			mtx.unlock();
		    } );
	    }
	    for( auto & i : threads ){
		i.join();
	    }
	    count = stack.size();
	    CHECK( 0 == count );
	    for( int i = 0; i < num_threads; ++i ){
		auto it = vals_retrieve.find(i);
		CHECK( vals_retrieve.end() != it );
		if( vals_retrieve.end() != it )
		    vals_retrieve.erase(it);
	    }
	}
    }
}
