template< typename T >
stack_lockfree_partial_elim_impl<T>::stack_lockfree_partial_elim_impl(){
    _head.store( nullptr );
}
template< typename T >
stack_lockfree_partial_elim_impl<T>::~stack_lockfree_partial_elim_impl(){
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
bool stack_lockfree_partial_elim_impl<T>::push( T const & val ){
    Node * new_node = new Node( val );
    return push_aux(new_node);
}
template< class T >
bool stack_lockfree_partial_elim_impl<T>::push( T && val ){
    Node * new_node = new Node( val );
    return push_aux(new_node);
}
template< class T >
bool stack_lockfree_partial_elim_impl<T>::push_aux( Node * new_node ){
    Node * head = _head.load( std::memory_order_relaxed );
    new_node->_next = head;
    while( !_head.compare_exchange_weak( new_node->_next, new_node, std::memory_order_release ) );
    return true;
}
template< class T >
std::optional<T> stack_lockfree_partial_elim_impl<T>::pop(){
    Node * head = _head.load( std::memory_order_relaxed );
    while( head && !_head.compare_exchange_weak( head, head->_next, std::memory_order_acquire ) );
    if( !head )
        return false;
    T val(head->_val);
    delete head;
    return std::optional<T>(val);
}
template< class T >
_t_size_t stack_lockfree_partial_elim_impl<T>::size() const {
    Node * current_node = _head.load( std::memory_order_relaxed );
    _t_size_t count = 0;
    while( current_node ){
        ++count;
        current_node = current_node->_next;
    }
    return count;
}
template< typename T >
bool stack_lockfree_partial_elim_impl<T>::empty() const {
    return size() == 0;
}
template< typename T >
bool stack_lockfree_partial_elim_impl<T>::clear(){
    while( !empty() ){
        T t;
        pop( t );
    }
    return true;
}
