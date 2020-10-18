//specialization for epoch based reclamation

#include <iostream>
#include "reclam_epoch.hpp"

template< typename T >
queue_lockfree_total_impl<T, trait_reclamation::epoch>::queue_lockfree_total_impl(){
    Node * sentinel = new Node();
    _head.store( sentinel );
    _tail.store( sentinel );
}
template< typename T >
queue_lockfree_total_impl<T, trait_reclamation::epoch>::~queue_lockfree_total_impl(){

    //must ensure there are no other threads accessing the datastructure
    
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
bool queue_lockfree_total_impl<T, trait_reclamation::epoch>::push_back( T && val ){ //push item to the tail
    Node * new_node = new Node( val );
    return push_back_aux(new_node);
}

template< typename T >
bool queue_lockfree_total_impl<T, trait_reclamation::epoch>::push_back( T const & val ){ //push item to the tail
    Node * new_node = new Node( val );
    return push_back_aux(new_node);
}

template< typename T >
bool queue_lockfree_total_impl<T, trait_reclamation::epoch>::push_back_aux( Node * new_node ){ //push item to the tail
    while( true ){

        auto guard = reclam_epoch<Node>::critical_section();
        
        Node * tail = _tail.load( std::memory_order_relaxed );
    
        if( nullptr == tail ){
            return false;
        }
                
        if( !_tail.compare_exchange_weak( tail, tail, std::memory_order_relaxed ) ){
            std::this_thread::yield();
            continue;
        }
        
        Node * tail_next = tail->_next.load( std::memory_order_relaxed );
        if( nullptr == tail_next ){  //determine if thread has reached tail
            if( tail->_next.compare_exchange_weak( tail_next, new_node, std::memory_order_acq_rel ) ){ //add new node
                _tail.compare_exchange_weak( tail, new_node, std::memory_order_relaxed ); //if thread succeeds, set new tail
                return true;
            }
        }else{
            _tail.compare_exchange_weak( tail, tail_next, std::memory_order_relaxed ); //update tail and retry
            std::this_thread::yield();
        }
    }
}
template< typename T >
std::optional<T> queue_lockfree_total_impl<T, trait_reclamation::epoch>::pop_front(){ //obtain item from the head
    while( true ){

        auto guard = reclam_epoch<Node>::critical_section();
        
        Node * head = _head.load( std::memory_order_relaxed );
    
        if( nullptr == head ){
            return std::nullopt;
        }

        if(!_head.compare_exchange_weak( head, head, std::memory_order_relaxed )){
            std::this_thread::yield();
            continue;
        }
        
        Node * tail = _tail.load( std::memory_order_relaxed );
        
        Node * head_next = head->_next.load( std::memory_order_relaxed );
    
        if(!_head.compare_exchange_weak( head, head, std::memory_order_relaxed )){
            std::this_thread::yield();
            continue;
        }

        if( head == tail ){
            if( nullptr == head_next ){//empty
                return std::nullopt;
            }else{
                _tail.compare_exchange_weak( tail, head_next, std::memory_order_relaxed ); //other thread updated head/tail, so retry
                std::this_thread::yield();
            }
        }else{
            //val = head_next->_val; //optimization: reordered to after exchange due to hazard pointer guarantees
            if( _head.compare_exchange_weak( head, head_next, std::memory_order_relaxed ) ){ //try add new item
                //thread suceeds
                T val(head_next->_val);
                reclam_epoch<Node>::retire(head);
                return std::optional<T>(val);
            }
        }
    }
}

template< typename T >
size_t queue_lockfree_total_impl<T, trait_reclamation::epoch>::size(){
    size_t count = 0;
    Node * node = _head.load();
    if( nullptr == node ){
        return 0;
    }
    while( node ){
        Node * next = node->_next.load(std::memory_order_relaxed);
        node = next;
        ++count;
    }
    return count - 1; //discount for sentinel node
}
template< typename T >
bool queue_lockfree_total_impl<T, trait_reclamation::epoch>::empty(){
    return size() == 0;
}
template< typename T >
bool queue_lockfree_total_impl<T, trait_reclamation::epoch>::clear(){
    size_t count = 0;
    while( !empty() ){
        pop_front();
        count++;
    }
    return true;
}
