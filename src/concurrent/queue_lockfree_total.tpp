template< typename T >
queue_lockfree_total_impl<T>::queue_lockfree_total_impl(){
    Node * sentinel = new Node();
    _head.store( sentinel );
    _tail.store( sentinel );
}
template< typename T >
queue_lockfree_total_impl<T>::~queue_lockfree_total_impl(){
    clear();
    if( _head ){
	Node * n = _head.load();
	if( _head.compare_exchange_strong( n, nullptr, std::memory_order_relaxed ) ){
	    if( n ){
		delete n;
		_head.store(nullptr);
		_tail.store(nullptr);
	    }
	}
    }
}
template< typename T >
bool queue_lockfree_total_impl<T>::push_back( T const & val ){ //push item to the tail
    Node * new_node = new Node( val );
    while( true ){
	Node * tail = _tail.load( std::memory_order_relaxed );
	if( nullptr == tail ){ //TODO: stricter check if _head/_tail is deallocated during destruction
	    return false;
	}
	Node * tail_next = tail->_next.load( std::memory_order_relaxed );
	if( nullptr == tail_next ){  //determine if thread has reached tail
	    if( tail->_next.compare_exchange_weak( tail_next, new_node, std::memory_order_relaxed ) ){ //add new node
		_tail.compare_exchange_weak( tail, new_node, std::memory_order_relaxed ); //if thread succeeds, set new tail
		return true;
	    }
	}else{
	    _tail.compare_exchange_weak( tail, tail_next, std::memory_order_relaxed ); //update tail and retry
	}
    }
}
template< typename T >
bool queue_lockfree_total_impl<T>::pop_front( T & val ){ //obtain item from the head
    while( true ){
	Node * head = _head.load( std::memory_order_relaxed );
	Node * tail = _tail.load( std::memory_order_relaxed );
	if( nullptr == head ){ //TODO: stricter check if _head/_tail is deallocated during destruction
	    return false;
	}
	Node * head_next = head->_next.load( std::memory_order_relaxed );
	if( head == _head ){
	    if( head == tail ){
		if( nullptr == head_next ){ //empty
		    return false;
		}else{
		    _tail.compare_exchange_weak( tail, head_next, std::memory_order_relaxed ); //other thread updated head/tail, so retry
		}
	    }else{
		val = head_next->_val;
		if( _head.compare_exchange_weak( head, head_next, std::memory_order_relaxed ) ){ //try add new item
		    delete head; //thread suceeds and returns
		    head = nullptr;
		    return true;
		}
	    }
	}
    }
}
template< typename T >
size_t queue_lockfree_total_impl<T>::size(){
    size_t count = 0;
    Node * node = _head.load();
    if( nullptr == node ){
	return 0;
    }
    while( node ){
	Node * next = node->_next.load();
	node = next;
	++count;
    }
    return count - 1; //discount for sentinel node
}
template< typename T >
bool queue_lockfree_total_impl<T>::empty(){
    return size() == 0;
}
template< typename T >
bool queue_lockfree_total_impl<T>::clear(){
    while( !empty() ){
	T t;
	pop_front( t );
    }
    return true;
}
