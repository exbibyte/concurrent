#include <iostream>
#include <cassert>

template< typename T >
queue_lockfree_simple_impl<T>::queue_lockfree_simple_impl(){
    Node * sentinel = new Node();
    _head.store( sentinel );
    _tail.store( sentinel );
}
template< typename T >
queue_lockfree_simple_impl<T>::~queue_lockfree_simple_impl(){
  
    clear();
    
    if( _head ){
        Node * n = _head.load();
        if( _head.compare_exchange_strong( n, nullptr ) ){
            if( n ){
                delete n;
                _head.store(nullptr);
                _tail.store(nullptr);
            }
        }
    }
}

template< typename T >
bool queue_lockfree_simple_impl<T>::push_back( Node * const node ){ //push item to the tail

    //make sure we don't insert bogus
    assert(node);
    assert( nullptr == node->_next.load( std::memory_order_relaxed ) );
    
    while( true ){
        
        Node * tail = _tail.load( std::memory_order_relaxed );
        
        if( nullptr == tail ){
            return false;
        }
        
        Node * tail_next = tail->_next.load( std::memory_order_relaxed );
        if( nullptr == tail_next ){  //determine if thread has reached tail
            if( tail->_next.compare_exchange_weak( tail_next, node, std::memory_order_relaxed ) ){ //add new node
                _tail.compare_exchange_weak( tail, node, std::memory_order_relaxed ); //if thread succeeds, set new tail
                return true;
            }
        }else{
            _tail.compare_exchange_weak( tail, tail_next, std::memory_order_relaxed ); //update tail and retry
        }
    }
}
template< typename T >
bool queue_lockfree_simple_impl<T>::pop_front( Node * & node ){ //obtain item from the head
    while( true ){
        Node * head = _head.load( std::memory_order_relaxed );
                
        Node * tail = _tail.load( std::memory_order_relaxed );
        if( nullptr == head ){
            return false;
        }
          
        Node * head_next = head->_next.load( std::memory_order_relaxed );
                
        if( _head.compare_exchange_weak( head, head, std::memory_order_relaxed ) ){
            if( head == tail ){
                if( nullptr == head_next ){ //empty
                    return false;
                }else{
                    _tail.compare_exchange_weak( tail, head_next, std::memory_order_relaxed ); //other thread updated head/tail, so retry
                }
            }else{
                T v = head_next->_val;
                if( _head.compare_exchange_weak( head, head_next, std::memory_order_relaxed ) ){
                    //head can be removed now
                    node = head;
                    node->_val = v; //return value
                    return true;
                }
            }
        }
    }
}
template< typename T >
typename queue_lockfree_simple_impl<T>::_t_size queue_lockfree_simple_impl<T>::size(){
    _t_size count = 0;
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
bool queue_lockfree_simple_impl<T>::empty(){
    return size() == 0;
}
template< typename T >
bool queue_lockfree_simple_impl<T>::clear(){
    while( !empty() ){
        Node * n = nullptr;
        if(pop_front( n )){
            if(n){
                delete n;
            }
        }
    }
    return true;
}
template< typename T >
void queue_lockfree_simple_impl<T>::for_each( std::function<void(Node * const)> f ){
  
    //disregard sentinel
  
    Node * node = _head.load(std::memory_order_acquire);
    Node * nn = node->_next.load(std::memory_order_acquire);
    while(!_head.compare_exchange_weak( node, node, std::memory_order_acquire )){
	node = _head.load(std::memory_order_acquire);
	nn = node->_next.load(std::memory_order_acquire);
    }
    
    while( nn ){
        f( nn );
        nn = nn->_next.load(std::memory_order_acquire);
    }  
}
