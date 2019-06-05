template< typename T >
stack_lockfree_total_simple_impl<T>::stack_lockfree_total_simple_impl(){
    _head.store( nullptr );
}
template< typename T >
stack_lockfree_total_simple_impl<T>::~stack_lockfree_total_simple_impl(){
    clear();
    if( _head ){
	Node * n = _head.load();
	if( n ){
	    delete n;
	    _head.store(nullptr);
	}
    }
}
template< class T >
bool stack_lockfree_total_simple_impl<T>::push( T const & val ){
    Node * new_node = new Node( val );
    Node * head = _head.load( std::memory_order_relaxed );
    new_node->_next = head;
    while( !_head.compare_exchange_weak( new_node->_next, new_node, std::memory_order_release ) );
    return true;
}
template< class T >
bool stack_lockfree_total_simple_impl<T>::pop( T & val ){
    Node * head = _head.load( std::memory_order_relaxed );
    while( head && !_head.compare_exchange_weak( head, head->_next, std::memory_order_acquire ) );
    if( !head )
        return false;
    val = head->_val;
    delete head;
    return true;
}
template< class T >
size_t stack_lockfree_total_simple_impl<T>::size() const {
    Node * current_node = _head.load( std::memory_order_relaxed );
    size_t count = 0;
    while( current_node ){
        ++count;
        current_node = current_node->_next;
    }
    return count;
}
template< typename T >
bool stack_lockfree_total_simple_impl<T>::empty() const {
    return size() == 0;
}
template< typename T >
bool stack_lockfree_total_simple_impl<T>::clear(){
    while( !empty() ){
	T t;
	pop( t );
    }
    return true;
}
