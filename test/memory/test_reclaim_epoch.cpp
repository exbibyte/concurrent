#define CATCH_CONFIG_MAIN

#include <vector>
#include <thread>
#include <iostream>
#include <mutex>
#include <cstdint>
#include <cassert>
#include <random>
#include <iostream>
#include <random>

#include "reclaim_epoch.hpp"

#include "catch.hpp"

struct Data {
    uint64_t p;
    uint64_t q;
    Data(){
	p = 0;
	q = 0;
    }
};

TEST_CASE( "epoch reclamation", "[epoch]" ) {

    std::vector<std::atomic<Data*>> arr_data(100);
    
    for(int i=0;i<100;++i){
	auto d = new Data();
	d->p = i;
	arr_data[i].store( d );
    }
    
    constexpr int numthread = 4;
    
    std::vector<std::thread> th;    

    std::mutex mtx_write;

    std::default_random_engine gen;
	
    auto f = [&](int id){
	reclaim_epoch<Data, numthread>::register_thread();

	std::uniform_int_distribution<int> distrib(0,3);
	std::uniform_int_distribution<int> distrib_arr(0,99);
	
	int count_read = 0;
	int count_removed = 0;
	
	std::vector<int> read;
	
	for(int i=0;i<300;++i){

	    int num = distrib(gen);
	    int arr_idx = distrib_arr(gen);
	    
	    if(num<3){
		
	    	auto guard = reclaim_epoch<Data, numthread>::read_guard();
	    	Data * ptr = arr_data[arr_idx].load(std::memory_order_relaxed);
	    	if(ptr){
	    	    auto val = ptr->p;
	    	    read.push_back(val);
	    	    ++count_read;
	    	}
	    }else{
	    	Data * ptr = arr_data[arr_idx].load(std::memory_order_relaxed);
	    	if(ptr){
		    //remove visibility of ptr from data array
		    if( arr_data[arr_idx].compare_exchange_strong( ptr, nullptr ) ){
			reclaim_epoch<Data, numthread>::retire(ptr); //defer memory reclamation
			++count_removed;
		    }
	    	}
	    }
	}

	{
	    std::lock_guard<std::mutex> lock(mtx_write);
	    std::cout << "count_read: " << count_read << ", count removed: " << count_removed << std::endl;
	    reclaim_epoch<Data, numthread>::deinit_thread();
	    reclaim_epoch<Data, numthread>::stat();
	}
    };
    
    for( int i = 0; i < numthread; ++i ){
	th.push_back( std::thread(f, i) );
    }
    
    for( auto & i: th ){
	i.join();
    }
    
    reclaim_epoch<Data, numthread>::drain_final();
    reclaim_epoch<Data, numthread>::stat();
	    
    int count_final_recycled = 0;
    for(auto &i: arr_data){
	auto d = i.load();
	if(d){
	    ++count_final_recycled;
	    delete d;
	}
    }
    std::cout << "count final recycled: " << count_final_recycled << std::endl;
}
