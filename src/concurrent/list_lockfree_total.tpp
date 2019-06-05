template< class T >
list_lockfree_total_impl< T >::list_lockfree_total_impl(){
    Node * h = new Node;
    h->_is_head = true;
    _head.store( h, std::memory_order_release );
}
template< class T >
list_lockfree_total_impl< T >::~list_lockfree_total_impl(){
    clear();
    Node * h = _head.load( std::memory_order_acquire );
    if( h )
	delete h;
}
template< class T >
bool list_lockfree_total_impl< T >::clear(){
    Node * n = _head.load( std::memory_order_acquire );
    n = n->_next.load( std::memory_order_acquire );
    while( n ){
	n->_is_marked = true;
	n = n->_next.load( std::memory_order_acquire );
    }
    n = _head.load( std::memory_order_acquire );
    n = n->_next.load( std::memory_order_acquire );
    while( n ){
	Node * remove = n;
	n = n->_next.load( std::memory_order_acquire );
	delete remove;
    }
    return false;
}
template< class T >
bool list_lockfree_total_impl< T >::empty(){
    return size() == 0;
}
template< class T >
size_t list_lockfree_total_impl< T >::size(){
    size_t count = 0;
    Node * n = _head.load( std::memory_order_acquire );
    n = n->_next.load( std::memory_order_acquire );
    while( n ){
	if( !n->_is_marked )
	    ++count;
	n = n->_next.load( std::memory_order_acquire );
    }
    return count;
}
template< class T >
bool list_lockfree_total_impl< T >::add( T const & val, size_t key ){
    while( true ){
	find_result window = list_lockfree_total_impl< T >::find_window( _head, key );
	Node * curr = window.second;
	if( curr && curr->_key == key ){
	    return false;
	}else{
	    Node * n = new Node( key, val );
	    n->_next = curr;
	    Node * prev = window.first;
	    if( prev->_next.compare_exchange_weak( curr, n, std::memory_order_acq_rel ) )
		return true;
	    else
		delete n;
	}
    }
}
template< class T >
bool list_lockfree_total_impl< T >::remove( T & val, size_t key ){
    while( true ){
	find_result window = list_lockfree_total_impl< T >::find_window( _head, key );
	Node * curr = window.second;
	Node * prev = window.first;
	if( curr && curr->_key != key ){
	    return false;
	}else{
	    if( !curr ){
		return false;
	    }
	    Node * succ = curr->_next.load( std::memory_order_acquire );
	    Node * remove = curr;
	    if( !prev->_next.compare_exchange_weak( curr, succ, std::memory_order_acq_rel ) ){
		//retry
		continue;
	    }
	    delete remove;
	    return true;
	}
    }
}
template< class T >
bool list_lockfree_total_impl< T >::contains( T const & val, size_t key ){
    Node * h = _head.load( std::memory_order_acquire );
    Node * n = h->_next.load( std::memory_order_acquire );
    while( n ){
	if( n->_key == key && !n->_is_marked && n->_val == val ){
	    return true;
	}else if( n->_key > key ){
	    return false;
	}else{
	    n = n->_next.load( std::memory_order_acquire );
	}
    }
    return false;
}
template< class T >
typename list_lockfree_total_impl< T>::find_result list_lockfree_total_impl< T >::find_window( _t_node & head, size_t key ){
    while( true ){
	Node * h = _head.load( std::memory_order_acquire );
	Node * n_prev = h;
	Node * n_curr = h->_next.load( std::memory_order_acquire );
	bool retry = false;
	while( n_curr ){
	    Node * n_succ = n_curr->_next.load( std::memory_order_acquire );
	    if( n_curr->_is_marked ){
		//remove node for garbage collection
		Node * n_to_remove = n_curr;
		if( false == n_prev->_next.compare_exchange_weak( n_to_remove, n_succ, std::memory_order_acq_rel ) ){
		    //retry
		    retry = true;
		    break;
		}
		delete n_to_remove;
	    }
	    if( n_curr->_key >= key ){
		return std::pair< Node *, Node * >( n_prev, n_curr );
	    }else{
		n_prev = n_curr;
		n_curr = n_succ;
	    }
	}
	if( retry )
	    continue;
	return std::pair< Node *, Node * >( n_prev, n_curr );
    }
}
