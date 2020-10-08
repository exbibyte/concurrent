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
#include <condition_variable>

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

    constexpr int num_data = 2;
    std::vector<std::atomic<Data*>> arr_data(num_data);
    
    for(int i=0;i<num_data;++i){
        auto d = new Data();
        d->p = i;
        arr_data[i].store( d );
    }
    
    int numthread = std::thread::hardware_concurrency();

    std::vector<std::thread> th;    

    std::mutex mtx_write;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine gen(seed);

    int sync_count = 0;

    std::mutex m_sync;
    std::condition_variable cv;

    std::map<int, int> removed_count;
    
    auto f = [&](int id){

        reclaim_epoch<Data>::register_thread();

        {
            std::scoped_lock<std::mutex> lock(m_sync);
            ++sync_count;
            cv.notify_all();
        }
        
        {
            std::unique_lock<std::mutex> lock(m_sync);
            cv.wait(lock, [&](){return sync_count>=numthread;});
        }
        
        std::uniform_int_distribution<int> distrib(0,3);
        std::uniform_int_distribution<int> distrib_arr(0,num_data-1);
    
        int count_read = 0;
        int count_removed = 0;
    
        std::vector<int> read;
        std::vector<int> removed;

        int chunk = num_data * 800000;
        for(int i=0;i<chunk;++i){

            int num = distrib(gen);
            int arr_idx = distrib_arr(gen);

            auto guard = reclaim_epoch<Data>::critical_section();

            if(num<=1){
                Data * ptr = arr_data[arr_idx].load(std::memory_order_acquire);
                if(ptr){
                    auto val = ptr->p;
                    read.push_back(val);
                    ++count_read;
                }
            }else{
                auto d_new = new Data();
                d_new->p = num_data + id * chunk + i;
                
                Data * ptr = arr_data[arr_idx].load(std::memory_order_relaxed);
                if(ptr && arr_data[arr_idx].compare_exchange_strong(ptr, d_new)){
                    removed.push_back(ptr->p);
                    reclaim_epoch<Data>::retire(ptr);
                    ++count_removed;
                }else{
                    delete d_new;
                }
            }
        }
        
        {
            std::scoped_lock<std::mutex> lock(m_sync);
            ++sync_count;
            cv.notify_all();
        }
        
        {
            std::unique_lock<std::mutex> lock(m_sync);
            cv.wait(lock, [&](){return sync_count==numthread*2;});
        }
        
        {
            std::lock_guard<std::mutex> lock(mtx_write);
            std::cout << "count_read: " << count_read << ", count removed: " << count_removed << std::endl;
        }

        {
            std::scoped_lock<std::mutex> lock(m_sync);
            reclaim_epoch<Data>::stat();
            for(auto i:removed){
                removed_count[i]++;
            }
        }

        reclaim_epoch<Data>::deinit_thread();
    };
    
    for( int i = 0; i < numthread; ++i ){
        th.push_back( std::thread(f, i) );
    }
    
    for( auto & i: th ){
        i.join();
    }
        
    int count_remain = 0;
    for(auto &i: arr_data){
        auto d = i.load();
        if(d){
            ++count_remain;
            delete d;
        }
    }
    std::cout << "count final remain: " << count_remain << std::endl;
    for(auto [key,count]: removed_count){
        if(count!=1) std::cout <<"count: " << count << std::endl;
        assert(count==1);
    }
}
