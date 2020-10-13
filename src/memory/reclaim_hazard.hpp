//--hazard pointer memory reclamation
//---used for preventing ABA memory change
//---uses a simple queue backing and stores temporary pointers until they are safe for reuse
#ifndef RECLAIM_HAZARD_HPP
#define RECLAIM_HAZARD_HPP

#include <atomic>
#include <cstdint>
#include <unordered_set>
#include <mutex>
#include <list>
#include <vector>

#include "queue_lockfree_simple.hpp"

//hazard record
template<class T>
struct Rec {
    std::atomic< T *> val;
    std::atomic< Rec<T> * > next;
    Rec(): val(nullptr), next( nullptr ) {}
    Rec( T * const val ): val(val), next( nullptr ) {}
};

template<class T>
class hazard_guard {
public:
    hazard_guard( Rec<T> * );
    hazard_guard( hazard_guard<T> && other );
    hazard_guard& operator=( hazard_guard<T> && other );
    ~hazard_guard();
    hazard_guard() : hazard(nullptr) {}
    void done() { ~hazard_guard(); }
    hazard_guard(hazard_guard<T> const &) = delete;
    hazard_guard& operator=(hazard_guard<T> const &) = delete;
private:
    Rec<T> * hazard;
};

template<class T>
class reclaim_hazard {
public:

    //list containing Rec<T>*
    using Records = typename queue_lockfree_simple<Rec<T>*>::Node;
  
    //construct from freelist if possible
    static T * new_from_recycled();

    //create scoped guard for beginning of hazard, automatically signals when out of scope
    static hazard_guard<T> add_hazard( T * );

    //mark for deletion (does deallocation)
    static void retire_hazard( T * );

    //per-thread deinit, necessary if any add_hazard/retire_hazard has been called
    static void thread_deinit();

    //should be called by the last thread remaining
    static void final_deinit();
    
private:

    static void scan();
    static void reuse( T * );

    //candidate hazards for deletion
    static thread_local std::vector<T*> hazards;

    //dump for remaining hazard for final deinit
    static std::unordered_set<T *> global_hazards;
  
    //no longer a hazard
    static thread_local std::list<T*> free_list;
  
    //hazards signaled from current thread
    static thread_local Records* hazards_signaled;
    static thread_local size_t count_hazards_signaled;
  
    //constants for recycling sweeps
    static constexpr size_t num_hazards = 1000; //recycles Rec's
    static constexpr size_t capacity_freelist = 1000; //recycles T's
    
    static constexpr bool can_recycle_rec = false;
    // static constexpr bool can_recycle_rec = true;
    
    //free list for Records (queue_lockfree_simple<Rec<T>*>::Node)
    static queue_lockfree_simple<Rec<T>*> records_free;

    //global publication list (queue_lockfree_simple<Rec<T>*>::Node) containing lists of Rec<T>*
    static queue_lockfree_simple<Rec<T>*> records_busy;

    //free list of Rec<T>* 
    static thread_local std::list<Rec<T>*> rec_free;

    //for final deinit
    static std::vector<Rec<T>*> rec_free_global;

    ///final deinit helper
    static std::mutex mutex_deinit_global_hazards;
    static std::mutex mutex_deinit_rec_free_global;
};

#include "reclaim_hazard.tpp"

#endif
