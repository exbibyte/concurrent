#include <vector>
#include <list>
#include <thread>
#include <cassert>
#include <memory>
#include <iostream>

template<typename T>
thread_local std::vector<T*> reclaim_hazard<T>::hazards {};

template<typename T>
thread_local std::list<T*> reclaim_hazard<T>::free_list {};

template<typename T>
thread_local typename reclaim_hazard<T>::Records * reclaim_hazard<T>::hazards_signaled = nullptr;

template<typename T>
thread_local size_t reclaim_hazard<T>::count_hazards_signaled = 0;

template<typename T>
std::unordered_set<T *> reclaim_hazard<T>::global_hazards;

template<typename T>
std::mutex reclaim_hazard<T>::mutex_deinit_global_hazards;

template<typename T>
std::mutex reclaim_hazard<T>::mutex_deinit_rec_free_global;

template<typename T>
queue_lockfree_simple<Rec<T>*> reclaim_hazard<T>::records_free;

template<typename T>
queue_lockfree_simple<Rec<T>*> reclaim_hazard<T>::records_busy;

template<typename T>
thread_local std::list<Rec<T>*> reclaim_hazard<T>::rec_free {};

template<typename T>
std::vector<Rec<T>*> reclaim_hazard<T>::rec_free_global {};

template<typename T>
hazard_guard<T>::hazard_guard( Rec<T> * r ){
    assert( r );
    hazard = r;
}

template<typename T>
hazard_guard<T>::~hazard_guard(){
    //signal hazard in global record to null when guard goes out of scope
    if( hazard ){
        hazard->val.store(nullptr, std::memory_order_release);
        hazard = nullptr;
    }
}

template<typename T>
hazard_guard<T>::hazard_guard( hazard_guard<T> && other ){
    hazard = other.hazard;
    other.hazard = nullptr;
}

template<typename T>
hazard_guard<T> & hazard_guard<T>::operator=( hazard_guard<T> && other ){
    if( hazard ){
        hazard->val.store(nullptr, std::memory_order_release);
    }
    hazard = other.hazard;
    other.hazard = nullptr;
    return *this;
}

template<typename T>
hazard_guard<T> reclaim_hazard<T>::add_hazard( T * t ){
  
    Records * n = nullptr;

    std::unordered_set<Records*> records_reinsert;
    
    if(nullptr==hazards_signaled){

        while(true){
            if( !records_free.pop_front( n ) ){
                //assign a new record to the thread if none exist
                n = new Records(); //sentinel node
                hazards_signaled = n;
                hazards_signaled->_val = new Rec<T>(); //sentinel rec
                hazards_signaled->_next.store(nullptr, std::memory_order_release);
                assert( records_busy.push_back(hazards_signaled) );
                break;
            }else{
                //existing record found
                assert( n );
                if( nullptr != n->_val ){
                    records_reinsert.insert(n);
                }else{
                    hazards_signaled = n;
                    hazards_signaled->_val = new Rec<T>(); //sentinel rec
                    hazards_signaled->_next.store(nullptr, std::memory_order_release);
                    assert( records_busy.push_back(hazards_signaled) );
                    break;
                }
            }
        }
    }

    for(auto &i: records_reinsert){
        i->_next.store(nullptr, std::memory_order_release);
        assert( records_free.push_back(i) );
    }
  
    assert(hazards_signaled);
    assert(hazards_signaled->_val);

    //records for the thread should be in records_busy now

    //try recycle rec
    if(count_hazards_signaled > num_hazards*2){
        Rec<T> * nn = hazards_signaled->_val->next.load( std::memory_order_acquire );
        Rec<T> * last_nonnull = nullptr;
        while(nn){
            T * v = nn->val.load( std::memory_order_acquire );
            if(nullptr!=v){
                last_nonnull = nn;
            } 
            nn = nn->next.load( std::memory_order_acquire );      
        }

        if(last_nonnull){
            Rec<T> * start_remove = last_nonnull->next.load( std::memory_order_acquire );
            if(start_remove){
                last_nonnull->next.store(nullptr, std::memory_order_release);
                assert( start_remove->val.load( std::memory_order_acquire ) == nullptr );
                size_t num_freed = 0;
                while( start_remove ){
                    --count_hazards_signaled;
                    ++num_freed;
                    assert( nullptr == start_remove->val.load( std::memory_order_acquire ) );
                    rec_free.push_back(start_remove);
                    start_remove = start_remove->next.load( std::memory_order_acquire );
                }
            }
        }
    }

    //try find an empty slot in threadlocal records
    Rec<T> * nn_found_free = nullptr;
    Rec<T> * nn = hazards_signaled->_val->next.load( std::memory_order_acquire );
    while(nn){
        T * v = nn->val.load( std::memory_order_acquire );
        if(nullptr==v){
            nn_found_free = nn;
            break;
        }
        nn = nn->next.load( std::memory_order_acquire );
    }

    Rec<T> * m = nullptr;
    if(nullptr!=nn_found_free){

        //empty slot found
    
        m = nn_found_free;
        m->val.store( t, std::memory_order_release );
    
        return hazard_guard<T>(m);
    
    }else{

        //try get rec from rec_free list
        static_assert(capacity_freelist>1);
        if(rec_free.size() < 0.5 * capacity_freelist){
            int l = std::max(rec_free.size() * 2, capacity_freelist);
            for(int i=0; i < l; ++i){
                rec_free.push_back(new Rec<T>());
            }
        }
        m = rec_free.front();
        rec_free.pop_front();
        assert( nullptr == m->val.load( std::memory_order_acquire ) );
        m->val.store( t, std::memory_order_release );
        m->next.store( nullptr, std::memory_order_release );

        ++count_hazards_signaled;
    
        //insert rec into threadlocal hazard signal list
    
        Rec<T> * r = hazards_signaled->_val->next.load( std::memory_order_acquire );
        m->next.store( r, std::memory_order_release );
        while( !hazards_signaled->_val->next.compare_exchange_weak( r, m ) ){
            r = hazards_signaled->_val->next.load( std::memory_order_acquire );
            m->next.store( r, std::memory_order_release );
        }
    
        return hazard_guard<T>(m);
    }
}

template<typename T>
void reclaim_hazard<T>::retire_hazard( T * t){
    if(t){
        hazards.push_back(t);
        if( hazards.size() > num_hazards ){ //assumes fixed number of hazards for now
            scan();
        }
    }
}

template<typename T>
void reclaim_hazard<T>::scan(){

    //collect global hazards
    std::unordered_set<T*> collect_hazards;
    auto f = [&collect_hazards]( Records * const records ){
        if(records){
            Rec<T> * r = records->_val;//sentinel
            assert(r);
            Rec<T> * n = r->next.load( std::memory_order_acquire );
            while(n){
                T * v = n->val.load( std::memory_order_acquire );
                if( v ){
                    collect_hazards.insert(v);
                }
                n = n->next.load( std::memory_order_acquire );
            }
        }
    };

    records_busy.for_each(f);
  
    //garbage collect freed hazards
    std::vector<T*> temp;
    std::swap(temp,hazards);
    std::unordered_set<T*> seen;    
    for(auto&i: temp){
        if(i && seen.count(i) == 0){
            if(collect_hazards.end() != collect_hazards.find(i)){
                hazards.push_back(i);
            }else{
                reuse(i);
            }
        }
    }
}

//dump into freelist freelist
template<typename T>
void reclaim_hazard<T>::reuse( T * t ){

    free_list.push_back(t);
  
    if( free_list.size() > 4 * capacity_freelist ){
        for( int i = 0; i < capacity_freelist; ++i ){
            T * tt = free_list.front();
            free_list.pop_front();
            delete tt;
        }
    }

    if(can_recycle_rec){
        if( rec_free.size() > 4 * capacity_freelist ){
            for( int i = 0; i < capacity_freelist; ++i ){
                Rec<T> * r = rec_free.front();
                rec_free.pop_front();
                delete r;
            }
        }
    }
}

//recycle from threadlocal freelist
template<typename T>
T * reclaim_hazard<T>::new_from_recycled(){
    if(free_list.empty()){
        return nullptr;
    }else{
        T * t = free_list.front();
        free_list.pop_front();
        return t;
    }
}

template<typename T>
void reclaim_hazard<T>::thread_deinit(){
  
    //collect global hazards
    std::unordered_set<T*> collect_hazards;
    auto f = [&collect_hazards]( Records * const records ){
        if(records){
            Rec<T> * r = records->_val;//sentinel
            if(r){
                Rec<T> * n = r->next.load( std::memory_order_acquire );
                while(n){
                    T * v = n->val.load( std::memory_order_acquire );
                    if( v ){
                        collect_hazards.insert(v);
                    }
                    n = n->next.load( std::memory_order_acquire );
                }
            }
        }
    };

    records_busy.for_each(f);

    //garbage collect freed hazards
    std::vector<T*> temp;
    std::swap(temp,hazards);
    for(auto&i: temp){
        if(i){
            if(collect_hazards.end() != collect_hazards.find(i)){
                hazards.push_back(i);
            }else{
                free_list.push_back(i);
            }
        }
    }

    //remove from global records

    std::unordered_set<Rec<T>*> collected_records;
    
    if( nullptr != hazards_signaled ){
    
        assert( hazards_signaled->_val );
        Rec<T> * n = hazards_signaled->_val->next.load(std::memory_order_acquire);
  
        while(!hazards_signaled->_val->next.compare_exchange_strong(n,nullptr)){
            n = hazards_signaled->_val->next.load(std::memory_order_acquire);
        }
  
        while( n ){
            collected_records.insert(n);
            n = n->next.load(std::memory_order_acquire);
        }
        hazards_signaled = nullptr;
    }

    size_t count_non_null_hazards = 0;

    for( auto & i: collected_records ){
        assert(i);
        T *  v = i->val.load( std::memory_order_acquire );
        if(nullptr!=v){
            ++count_non_null_hazards;
        }
        i->next.store(nullptr, std::memory_order_release );
        std::lock_guard<std::mutex> l(mutex_deinit_rec_free_global);
        rec_free_global.push_back(i);
    }
    collected_records.clear();

    //maybe not needed
    for( auto & i: rec_free ){
        if(i) delete i;
        // std::lock_guard<std::mutex> l(mutex_deinit_rec_free_global);
        // rec_free_global.push_back(i);
    }
    rec_free.clear();

    //# of active hazards signaled by current thread should be zero
    assert( 0 == count_non_null_hazards );

    //deallocate T's
    for( auto & i : free_list ){
        if(i){
            delete i;
        }
    }
    free_list.clear();

    //defer deallocating remaining T's
    for(auto&i: hazards){
        std::lock_guard<std::mutex> l(mutex_deinit_global_hazards);
        global_hazards.insert(i);
    }
    hazards.clear();

    assert( hazards.empty() );
    assert( free_list.empty() );
    assert( nullptr == hazards_signaled );
    assert( rec_free.empty() );
}

template<typename T>
void reclaim_hazard<T>::final_deinit(){
  
    //deallocate T's
    {
        std::lock_guard<std::mutex> l(mutex_deinit_global_hazards);
        for(auto&i:global_hazards){
            assert(nullptr!=i);
            delete i;
        }
        global_hazards.clear();
    }
  
    //deallocate Rec's
    {
        std::lock_guard<std::mutex> l(mutex_deinit_rec_free_global);
        for(auto& i: rec_free_global ){
            assert(i);
            delete i;
        }
        rec_free_global.clear();
    }

    //deallocate Records and sentinel Rec's
    while( records_free.size() > 0 ){
        Records * n = nullptr;
        if( records_free.pop_front(n) ){
            assert(n);
            Rec<T> * nn = n->_val;
            while(nullptr!=nn){
                Rec<T> * nnn = nn->next.load( std::memory_order_acquire );
                delete nn;
                nn = nnn;
            }
            delete n;
        }
    }

    while( records_busy.size() > 0 ){
        Records * n = nullptr;
        if( records_busy.pop_front(n) ){
            assert(n);
            Rec<T> * nn = n->_val;
            while(nullptr!=nn){
                Rec<T> * nnn = nn->next.load( std::memory_order_acquire );
                delete nn;
                nn = nnn;
            }
            delete n;
        }
    }

    assert( global_hazards.empty() );
    assert( records_free.empty() );
    assert( records_busy.empty() );
    assert( rec_free_global.empty() );
}
