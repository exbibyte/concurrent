#ifndef STRESS_HASHTABLE_HPP
#define STRESS_HASHTABLE_HPP

#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

class stress_hashtable {
public:
    template< typename Hashtable >
    static void stress_put_get_int( unsigned int num_threads, Hashtable & hashtable ){
	int count_loop = 5;
	while( --count_loop >=0 ){
	    // std::cout << "insertion starts." << std::endl;
	    std::vector<std::thread> threads3( num_threads );
	    std::vector<std::thread> threads2( num_threads );
	    std::vector<std::thread> threads( num_threads );
	    auto t0 = std::chrono::high_resolution_clock::now();
	    for( int i = 0; i < num_threads; ++i ){
		threads[i] = std::thread( [ &, i ](){
			hashtable.insert( i, i );
		    } );
	    }
	    for( auto & i : threads )
	    	i.join();
	    auto t1 = std::chrono::high_resolution_clock::now();
	    // std::cout << "insertion ends." << std::endl;
	    // std::cout << "find starts." << std::endl;
	    std::vector<int> vec_count_found( num_threads, 0 );
	    for( int i = 0; i < num_threads; ++i ){
	        threads2[i] = std::thread( [&vec_count_found, &hashtable, i ](){
			int val_query;
			bool bRet = hashtable.find( i, val_query );
			if( bRet && (val_query == i) ){
			    vec_count_found[i] = 1;
			}
		    } );
	    }
	    for( auto & i : threads2 )
	    	i.join();
	    auto t2 = std::chrono::high_resolution_clock::now();
	    int count_found = 0;
	    for( auto i : vec_count_found )
		count_found += i;
	    // std::cout << "find ends." << std::endl;
	    // std::cout << "erase starts." << std::endl;
	    for( int i = 0; i < num_threads; ++i ){
	        threads3[i] = std::thread( [ &hashtable, i ](){
	    		bool bret = hashtable.erase( i );
			if( !bret ){
			    std::cout << "erasing failed. key: " << i << std::endl;
			    assert( 0 );
			}
	    	    } );
	    }
	    for( auto & i : threads3 )
	    	i.join();
	    auto t3 = std::chrono::high_resolution_clock::now();
	    // std::cout << "erase ends." << std::endl;
	    int count_del = 0;
	    for( int i = 0; i < num_threads; ++i ){
	    	int val_query;
	    	bool bRet = hashtable.find( i, val_query );
	    	if( !bRet ){
	    	    ++count_del;
	    	}else{
		    std::cout << "val exists: " << i << ", val: " << val_query << std::endl;
		    assert( 0 && "a value is left after expected erasure.");
		}
	    }
	    int v = 0;
	    for( auto & y : hashtable._locks ){
	    	bool bret = y.is_free();
	    	if( !bret ){
	    	    std::cout << "lock " << v << " not free. count: " << y.count() << std::endl;
	    	    assert( 0 && "a striped lock is not free after expected resource release." );
	    	}
	    	++v;
	    }
	    std::chrono::duration<double> dur_insert = t1 - t0;
	    std::chrono::duration<double> dur_find = t2 - t1;
	    std::chrono::duration<double> dur_del = t3 - t2;
	    auto dur_ms_ins = std::chrono::duration_cast<std::chrono::milliseconds>(dur_insert);
	    auto dur_ms_find = std::chrono::duration_cast<std::chrono::milliseconds>(dur_find);
	    auto dur_ms_del = std::chrono::duration_cast<std::chrono::milliseconds>(dur_del);
	    std::cout << "number of threads to insert: " << num_threads << ", count found: " << count_found << ", count deleted: " << count_del << ", rate insert: " << (double)num_threads/dur_ms_ins.count()*1000.0 << " /sec., rate find: " << (double)num_threads/dur_ms_find.count()*1000.0 << " /sec., rate erase: " << (double)num_threads/dur_ms_del.count()*1000.0 <<  " /sec." << std::endl;
	}
    }
};

#endif
