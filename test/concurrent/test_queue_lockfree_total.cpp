#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>

#include "catch.hpp"
#include "queue_lockfree_total.hpp"

using namespace std;

TEST_CASE( "queue_lockfree_total", "[queue push pop]" ) { 

    SECTION( "push-pop" ) {
    
	queue_lockfree_total<int> queue;
            
	size_t count = queue.size();
	CHECK( 0 == count );
	int val = 5;
	queue.put(val);
	count = queue.size();
	CHECK( 1 == count );

	int retrieve;
	bool bRet = queue.get( retrieve );
	count = queue.size();
	CHECK( 0 == count );
	CHECK( true == bRet );
	CHECK( 5 == retrieve );
	
	queue_lockfree_total<int>::thread_deinit();
    }
    SECTION( "pop_empty" ) {
    
	queue_lockfree_total<int> queue;
        
	int retrieve;
	size_t count;
	bool bRet = queue.get( retrieve );
	count = queue.size();
	CHECK( 0 == count );
	CHECK( false == bRet );
	
	queue_lockfree_total<int>::thread_deinit();
    }
    SECTION( "multiple instances" ) {
    
	queue_lockfree_total<int> q1,q2;
            
	CHECK( 0 == q1.size() );
	CHECK( 0 == q2.size() );
    
	q1.put(5);
	q2.put(6);

	CHECK( 1 == q1.size() );
	CHECK( 1 == q2.size() );

	int r1,r2;
      
	bool b1 = q1.get( r1 );
	CHECK( 0 == q1.size() );
	bool b2 = q2.get( r2 );

	CHECK( b1 );
	CHECK( 5 == r1 );
	CHECK( b2 );
	CHECK( 6 == r2 );

	CHECK( 0 == q1.size() );
	CHECK( 0 == q2.size() );
    
	queue_lockfree_total<int>::thread_deinit();
    }
}

TEST_CASE( "queue_lockfree_total_multithread", "[queue multithread]" ) { 
        
    SECTION( "multi-thread push-pop" ) {

	queue_lockfree_total<int> queue;

	int count_loop = 10;
	while( --count_loop >=0 ){
	    size_t count;
	    unsigned int num_threads = 50;
	    vector<thread> threads( num_threads );
	    for( int i = 0; i < num_threads; ++i ){
		threads[i] = std::thread( [ &, i ](){
			int val = i;
			queue.put( val );
			queue_lockfree_total<int>::thread_deinit();
		    } );
	    }
	    // count = queue.size();
	    // cout << "queue size after push threads started: " << count << endl;

	    vector<thread> threads2( num_threads );
	    set<int> vals_retrieve;
	    for( int i = 0; i < num_threads * 0.1; ++i ){
		threads2[i] = std::thread( [&](){
			int pop_val;
			bool bRet = queue.get( pop_val );
			if( bRet ){
			    // std::cout << pop_val << std::endl;
			}
			queue_lockfree_total<int>::thread_deinit();
		    } );
	    }
	    for( int i = 0; i < num_threads * 0.1; ++i ){
		threads2[i].join();
	    }
	    for( auto & i : threads ){
		i.join();
	    }
	    count = queue.size();
      
	    CHECK( ((num_threads * 0.9 ) * 0.8) <= count );
	    CHECK( ((num_threads * 0.9 ) * 1.2) >= count );
      
	    for( int i = 0; i < num_threads * 0.9; ++i ){
		threads2[i] = std::thread( [&](){
			int pop_val;
			bool bRet = queue.get( pop_val );
			if( bRet ){
			    // std::cout << pop_val << std::endl;
			}
			queue_lockfree_total<int>::thread_deinit();
		    } );
	    }
	    for( int i = 0; i < num_threads * 0.9; ++i ){
		threads2[i].join();
	    }
	    count = queue.size();
      
	    CHECK( 0 <= count );
	    CHECK( (num_threads * 0.1 ) >= count );
	}
	queue_lockfree_total<int>::thread_deinit();
    }
}

TEST_CASE( "queue_lockfree_total_multithread_long_lived", "[queue multithread longlived]" ) { 
	
    queue_lockfree_total<int> queue;

    size_t nums = 1000000;
    
    vector<int> retrieved( nums, 0);
    
    unsigned int num_threads = 4;

    auto t0 = std::chrono::high_resolution_clock::now();
	    
    vector<thread> threads( num_threads );
    for( int i = 0; i < num_threads; ++i ){
	threads[i] = std::thread( [ &, i ](){
		int val = nums/num_threads*i;
		for( int j = 0; j < nums/num_threads; ++j ){
		    while( !queue.put( val + j ) ){} //force enqueue
		}
		queue_lockfree_total<int>::thread_deinit();
	    } );
    }

    vector<thread> threads2( num_threads );
    for( int i = 0; i < num_threads; ++i ){
	threads2[i] = std::thread( [&](){
		for( int j = 0; j < nums/num_threads; ++j ){
		    int pop_val;
		    while( !queue.get( pop_val ) ){} //force dequeue
		    ++retrieved[pop_val];
		}
		queue_lockfree_total<int>::thread_deinit();
	    } );
    }
  
    for( auto & i : threads ){
	i.join();
    }
    for( auto & i : threads2 ){
	i.join();
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = t1 - t0;
    auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
    std::cout << "duration: " << dur_ms.count() << " ms. rate: " <<  (double)nums/dur_ms.count()*1000.0 << " put-get/sec." << std::endl;
    
    size_t count = queue.size();

    CHECK( 0 == count );

    int k = 0;
    int n = 0;
    //expect count of 1 for each retrieved number
    for(auto i: retrieved){
	if(i!=1){
	    //oops, something is wrong, write out count
	    ++k;
	    std::cout << n << ": " << i << std::endl;
	}
	++n;
    }
    CHECK( 0 == k );  
}

