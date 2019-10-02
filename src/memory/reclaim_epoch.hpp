#ifndef RECLAIM_EPOCH_HPP
#define RECLAIM_EPOCH_HPP

#include <atomic>
#include <thread>
#include <cstdint>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "queue_lockfree_simple.hpp"

template<class T, unsigned int N>
class epoch_guard {
public:
    
    epoch_guard(std::atomic<bool> & flag): flag_critical(&flag){
        flag_critical->store(true, std::memory_order_release);
    }
    ~epoch_guard(){
	flag_critical->store(false, std::memory_order_release);
    }
    epoch_guard( epoch_guard<T,N> && other ){
        flag_critical = other.flag_critical;
	other.flag_critical = nullptr;
    }

    //not copy constructable / copy assignable
    epoch_guard() = delete;
    epoch_guard(epoch_guard const &) = delete;
    epoch_guard& operator=(epoch_guard const &) = delete;
    
    std::atomic<bool> * flag_critical;
};

template<class T, unsigned int N>
class reclaim_epoch {
public:
    
    using _t_data = T;

    static constexpr uint64_t ticker_thresh = 30;
    static constexpr size_t local_node_thresh = 20;
    
    static void register_thread();

    //remove per-thread resources
    static void deinit_thread();

    //final sweep to clear remaining garbage after all threads are done
    static void drain_final();

    static std::atomic<bool> flags_critical[N];
    
    static epoch_guard<T, N> read_guard();

    static void retire( T * );
    
    static void stat();
    
private:

    static void collect_garbage();
    
    static thread_local uint8_t id;
    static thread_local size_t idx;
    
    static thread_local uint64_t local_ticker;

    static thread_local std::vector<typename queue_lockfree_simple<T*>::Node*> local_nodes;
    
    static std::atomic<uint8_t> global_epoch;
    
    static std::atomic<uint8_t> local_epochs[N];

    static queue_lockfree_simple<T*> epoch_garbage[3];

    static std::unordered_map<size_t,size_t> thread_ids;
    static std::mutex mutex_thread_ids;

    static int threads_register_count;

    static thread_local size_t count_recycled;
};

#include "reclaim_epoch.tpp"

#endif
