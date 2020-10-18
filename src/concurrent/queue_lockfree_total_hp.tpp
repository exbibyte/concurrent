//specialization for hazard pointer reclamation

#include <iostream>
#include "reclam_hazard.hpp"

template< typename T >
queue_lockfree_total_impl<T, trait_reclamation::hp>::queue_lockfree_total_impl(){
    Node * sentinel = new Node();
    _head.store( sentinel );
    _tail.store( sentinel );
}
template< typename T >
queue_lockfree_total_impl<T, trait_reclamation::hp>::~queue_lockfree_total_impl(){

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
bool queue_lockfree_total_impl<T, trait_reclamation::hp>::push_back( T && val ){ //push item to the tail

    if(auto existing = reclam_hazard<Node>::new_from_recycled()){
        existing->_val = std::move(val);
        existing->_next = nullptr;
        return push_back_aux(existing);        
    }else{
        Node * new_node = new Node( val );
        return push_back_aux(new_node);
    }
}

template< typename T >
bool queue_lockfree_total_impl<T, trait_reclamation::hp>::push_back( T const & val ){ //push item to the tail
    if(auto existing = reclam_hazard<Node>::new_from_recycled()){
        existing->_val = val;
        existing->_next = nullptr;
        return push_back_aux(existing);        
    }else{
        Node * new_node = new Node( val );
        return push_back_aux(new_node);
    }
}

template< typename T >
bool queue_lockfree_total_impl<T, trait_reclamation::hp>::push_back_aux( Node * new_node ){ //push item to the tail
    while( true ){
        Node * tail = _tail.load( std::memory_order_relaxed );

        if(!tail) return false;
        
        hazard_guard<Node> guard = reclam_hazard<Node>::add_hazard( tail );

        Node * tail_next = tail->_next.load( std::memory_order_relaxed );

        hazard_guard<Node> guard2 = reclam_hazard<Node>::add_hazard( tail_next );

        if( nullptr == tail_next ){  //determine if thread has reached tail
            if( tail->_next.compare_exchange_weak( tail_next, new_node, std::memory_order_acq_rel ) ){ //add new node
                _tail.compare_exchange_weak( tail, new_node, std::memory_order_relaxed ); //if thread succeeds, set new tail
                return true;
            }else{
                std::this_thread::yield();
            }    
        }else{
            _tail.compare_exchange_weak( tail, tail_next, std::memory_order_relaxed ); //update tail and retry
            std::this_thread::yield();
        }
    }
}
template< typename T >
std::optional<T> queue_lockfree_total_impl<T, trait_reclamation::hp>::pop_front(){ //obtain item from the head
    
    while( true ){
        
        Node * head = _head.load( std::memory_order_relaxed );

        if(!head) return std::nullopt;
        
        hazard_guard<Node> guard1 = reclam_hazard<Node>::add_hazard( head );
        
        Node * tail = _tail.load( std::memory_order_relaxed );

        Node * head_next = head->_next.load( std::memory_order_relaxed );

        hazard_guard<Node> guard2 = reclam_hazard<Node>::add_hazard(head_next);
        
        if( head == tail ){
            if( nullptr == head_next ){//empty
                return std::nullopt;
            }else{
                _tail.compare_exchange_weak( tail, head_next, std::memory_order_relaxed ); //other thread updated head/tail, so retry
                std::this_thread::yield();
            }
        }else{
            //val = std::move(head_next->_val); //optimization: reordered to after exchange due to hazard pointer guarantees
            if( _head.compare_exchange_weak( head, head_next, std::memory_order_acq_rel ) ){ //try add new item
                //thread suceeds
                T val(std::move(head_next->_val));
                reclam_hazard<Node>::retire_hazard(head);
                return std::optional<T>(val);
            }else{
                std::this_thread::yield();
            }
        }
    }
}

template< typename T >
size_t queue_lockfree_total_impl<T, trait_reclamation::hp>::size(){
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
bool queue_lockfree_total_impl<T, trait_reclamation::hp>::empty(){
    return size() == 0;
}
template< typename T >
bool queue_lockfree_total_impl<T, trait_reclamation::hp>::clear(){
    size_t count = 0;
    while( !empty() ){
        pop_front();
        count++;
    }
    return true;
}
