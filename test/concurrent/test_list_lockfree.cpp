#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>

#include "catch.hpp"
#include "list_lockfree_total.hpp"

using namespace std;

TEST_CASE( "list_lockfree_total single thread", "[list total]" ) { 
    list_lockfree_total<int,trait_reclamation::not_applicable> l;
    CHECK( l.size() == 0 );
    CHECK( l.empty() == true );
    size_t n = 20;
    for( size_t i = 0; i < n; ++i ){
	int val = i;
	bool bret = l.add( val, i );
	CHECK( bret );
    }
    CHECK( l.size() == n );
    CHECK( l.empty() == false );
    for( size_t i = 0; i < n; ++i ){
	int val = i;
	bool bret = l.contains( val, i );
	CHECK( bret );
    }
    for( size_t i = 0; i < n; ++i ){
	int val = i;
	bool bret = l.remove( val, i );
	CHECK( bret );
    }
    CHECK( l.size() == 0 );
    CHECK( l.empty() == true );
}
