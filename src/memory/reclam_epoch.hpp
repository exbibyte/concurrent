#ifndef RECLAM_EPOCH_HPP
#define RECLAM_EPOCH_HPP

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

template<class T>
class reclam_epoch {
public:
    
    using _t_data = T;

    static constexpr int FREQ_RECYCLE = 10;
    static constexpr int FREQ_EPOCH = 10;

    static constexpr uint64_t EPOCH_DEFAULT = std::numeric_limits<uint64_t>::max();

    class epoch_guard {
    public:
        epoch_guard(std::atomic<uint64_t> * f): flag(f){}
        ~epoch_guard(){
            // flag->store(EPOCH_DEFAULT, std::memory_order_release);
            done();
        }
        epoch_guard( epoch_guard && other ){
            flag = other.flag;
            other.flag = nullptr;
        }
        void done(){
            flag->store(EPOCH_DEFAULT, std::memory_order_relaxed);
        }
        epoch_guard() = delete;
        epoch_guard(epoch_guard const &) = delete;
        epoch_guard& operator=(epoch_guard const &) = delete;
        std::atomic<uint64_t> * flag;
    };

    static void thread_init(){
        static reclam_global_obj _g_global_obj;
        register_thread();
    }
    static void thread_deinit(){
        recycle_final();
        //todo: add safe thread unregistering
    }
    
    static void register_thread();
    static void unregister_thread();
    static void deinit_thread();
    static int sync();
    static epoch_guard critical_section();
    static void retire(T *);

    static void stat();

    static void clear_epoch_list();

private:

    static void recycle();
    static void recycle_final();

    static thread_local int idx;
    
    static thread_local std::atomic<uint64_t> epoch_local;
    
    static thread_local std::vector<std::pair<T *,uint64_t>> local_recycle;

    static std::atomic<uint64_t> freq_epoch;

    static thread_local int local_freq_recycle;
    
    static std::atomic<uint64_t> epoch_global;

    static queue_lockfree_simple<std::atomic<uint64_t>*> epoch_list;

    static thread_local int count_recycled; //debugging only

    class reclam_global_obj {
    public:
        ~reclam_global_obj(){
#ifdef DEBUG_VERBOSE
            std::cout << "reclam global obj destructor" << std::endl;
#endif
            reclam_epoch::clear_epoch_list();
        }
    };

};

#include "reclam_epoch.tpp"

#endif
