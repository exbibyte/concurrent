template<class T, unsigned int N>
std::atomic<uint8_t> reclaim_epoch<T, N>::global_epoch(0);

template<class T, unsigned int N>
thread_local uint64_t reclaim_epoch<T, N>::local_ticker = 0;

template<class T, unsigned int N>
std::atomic<uint8_t> reclaim_epoch<T, N>::local_epochs[N] = {};

template<class T, unsigned int N>
queue_lockfree_simple<T*> reclaim_epoch<T, N>::epoch_garbage[3];

template<class T, unsigned int N>
thread_local uint8_t reclaim_epoch<T, N>::id;

template<class T, unsigned int N>
thread_local size_t reclaim_epoch<T, N>::idx = -1;

template<class T, unsigned int N>
int reclaim_epoch<T, N>::threads_register_count = 0;

template<class T, unsigned int N>
std::atomic<bool> reclaim_epoch<T, N>::flags_critical[N] = {};

template<class T, unsigned int N>
std::mutex reclaim_epoch<T, N>::mutex_thread_ids;

template<class T, unsigned int N>
std::unordered_map<size_t,size_t> reclaim_epoch<T, N>::thread_ids;

template<class T, unsigned int N>
thread_local size_t reclaim_epoch<T, N>::count_recycled = 0;

template<class T, unsigned int N>
thread_local std::vector<typename queue_lockfree_simple<T*>::Node*> reclaim_epoch<T, N>::local_nodes;

template<class T, unsigned int N>
epoch_guard<T,N> reclaim_epoch<T, N>::read_guard(){
	
    local_epochs[reclaim_epoch<T, N>::idx] = global_epoch.load(std::memory_order_seq_cst);
	
    if( ++local_ticker > ticker_thresh ){
	local_ticker = 0;
	collect_garbage();
    }
	
    return epoch_guard<T,N>( flags_critical[idx] );
}

template<class T, unsigned int N>
void reclaim_epoch<T, N>::collect_garbage(){
    
    bool equal = true;

    uint8_t current_epoch = global_epoch.load(std::memory_order_seq_cst);
	
    for(int i = 0; i < N; ++i){
	equal &= (local_epochs[i] == current_epoch) && (false == flags_critical[i]);
    }
	
    uint8_t next_epoch = (current_epoch+1)%3;
    uint8_t prev_epoch = (current_epoch+3-1)%3;
    
    if(equal && global_epoch.compare_exchange_strong(current_epoch, next_epoch) ){

	typename queue_lockfree_simple<T*>::Node * n;
	while( epoch_garbage[prev_epoch].pop_front( n ) ){
	    if(n->_val != nullptr){
		++count_recycled;
		delete n->_val;
	    }
	    assert(n != nullptr);
	    n->_val = nullptr;
	    n->_next = nullptr;
	    local_nodes.push_back(n);
	    
	    if( local_nodes.size() > local_node_thresh ){
		// delete n;
		auto it = local_nodes.begin()+local_nodes.size()/2;
		for(auto it_a = local_nodes.begin(); it_a != it; ++it_a){
		    delete *it_a;
		}
		auto temp = std::vector<typename queue_lockfree_simple<T*>::Node*>(it, local_nodes.end());
		std::swap(temp,local_nodes);
	    }
	}
    }    
}

template<class T, unsigned int N>
void reclaim_epoch<T, N>::retire( T * obj ){

    local_epochs[reclaim_epoch<T, N>::id] = global_epoch.load(std::memory_order_seq_cst);
    
    if( ++local_ticker > ticker_thresh ){
	local_ticker = 0;
	collect_garbage();
    }
    
    if(obj){
	
	typename queue_lockfree_simple<T*>::Node * n = nullptr;
	
	if(local_nodes.empty()){
	    n = new typename queue_lockfree_simple<T*>::Node(obj);
	}else{
	    n = local_nodes.back();
	    n->_val = obj;
	    local_nodes.pop_back();
	}
	
	assert(n != nullptr);
	
	epoch_garbage[local_epochs[idx]].push_back(n);
    }
}

template<class T, unsigned int N>
void reclaim_epoch<T, N>::register_thread(){
    {
	std::lock_guard<std::mutex> g (mutex_thread_ids);
	idx = thread_ids.size();
	thread_ids[(size_t)&reclaim_epoch<T, N>::id] = idx;
	threads_register_count++;
    }

    while(threads_register_count!=N){
	std::this_thread::yield();
    }
}

template<class T, unsigned int N>
void reclaim_epoch<T, N>::deinit_thread(){
    for(auto i: local_nodes){
	assert(i);
	delete i;
    }
    local_nodes.clear();
}

template<class T, unsigned int N>
void reclaim_epoch<T, N>::drain_final(){

    for(int i=0; i<3; ++i){
	epoch_garbage[i].for_each(
	    [](auto n){
		if(n->_val) {
		    delete n->_val;
		    ++count_recycled;
		}
	    });
    }
    
}

template<class T, unsigned int N>
void reclaim_epoch<T, N>::stat(){
    std::cout << "count recycled: " << count_recycled << std::endl;
}
