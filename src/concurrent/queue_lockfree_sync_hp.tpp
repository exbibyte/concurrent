#include <iostream>
#include <cassert>

template< typename T >
queue_lockfree_sync_impl<T, trait_reclamation::hp>::queue_lockfree_sync_impl(){
    Node * sentinel = new Node();
    sentinel->_type.store( NodeType::SENTINEL, std::memory_order_release );
    _head.store( sentinel );
    _tail.store( sentinel );
}
template< typename T >
queue_lockfree_sync_impl<T, trait_reclamation::hp>::~queue_lockfree_sync_impl(){
    clear();
    Node * n = _head.load();
    auto guard = reclam_hazard<Node>::add_hazard(n);
    if( _head.compare_exchange_strong( n, nullptr ) ){
        if( n ){
            reclam_hazard<Node>::retire_hazard(n);
            _head.store(nullptr);
            _tail.store(nullptr);
        }
    }
}

template< typename T >
bool queue_lockfree_sync_impl<T, trait_reclamation::hp>::push_back( T const & val ){ //push an item to the tail
    Node * new_node = new Node( val ); //type is ITEM if value argument is present
    return push_back_aux(new_node);
}

template< typename T >
bool queue_lockfree_sync_impl<T, trait_reclamation::hp>::push_back( T && val ){ //push an item to the tail
    Node * new_node = new Node( val ); //type is ITEM if value argument is present
    return push_back_aux(new_node);
}

template< typename T >
bool queue_lockfree_sync_impl<T, trait_reclamation::hp>::push_back_aux( Node * new_node ){ //push an item to the tail
    while( true ){
        Node * tail = _tail.load( std::memory_order_acquire );
        Node * head = _head.load( std::memory_order_acquire );
        if( nullptr == head || nullptr == tail || head->_type.load( std::memory_order_relaxed ) != NodeType::SENTINEL ){
            std::this_thread::yield();
            continue;
        }

        auto guard = reclam_hazard<Node>::add_hazard(tail);
        
        Node * n = head->_next.load( std::memory_order_relaxed );
        NodeType tail_type = tail->_type.load( std::memory_order_relaxed );
        if( NodeType::ITEM == tail_type || NodeType::SENTINEL == tail_type ){ //try enque an item by putting an ITEM object in queue
            Node * tail_next = tail->_next.load( std::memory_order_relaxed );
            auto guard2 = reclam_hazard<Node>::add_hazard(tail_next);
            if( tail == _tail.load(std::memory_order_relaxed) ){
                if( nullptr != tail_next ){ //tail is invalidated
                    _tail.compare_exchange_weak( tail, tail_next, std::memory_order_relaxed ); //update tail before retry
                }else if( tail->_next.compare_exchange_weak( tail_next, new_node, std::memory_order_relaxed ) ){ //try commit item as the next node
                    _tail.compare_exchange_weak( tail, new_node, std::memory_order_relaxed ); //try update tail after commit
                    //wait for synchronization with dequing thread for the signal that transaction is complete
                    while( new_node->_type.load( std::memory_order_acquire ) != NodeType::FULFILLED ){
                        std::this_thread::yield();
                    }
                    new_node->_type.store( NodeType::COMPLETE, std::memory_order_release ); //signal for cleanup of the completed node
                    //performance optimization starts
                    Node * current_head = _head.load( std::memory_order_acquire);
                    auto guard3 = reclam_hazard<Node>::add_hazard(current_head);
                    if( nullptr != current_head && current_head->_type.load( std::memory_order_acquire ) == NodeType::SENTINEL ){
                        Node * recycle = current_head;
                        Node * current_head_next = current_head->_next.load( std::memory_order_acquire );
                        auto guard4 = reclam_hazard<Node>::add_hazard(current_head_next);
                        if( current_head_next ){
                            if( current_head_next->_type.load( std::memory_order_acquire ) == NodeType::COMPLETE ){
                                if( _head.compare_exchange_weak( current_head, current_head_next, std::memory_order_acq_rel ) ){
                                    current_head_next->_type.store( NodeType::SENTINEL, std::memory_order_release );
                                    reclam_hazard<Node>::retire_hazard(recycle);
                                    recycle = nullptr;
                                }
                            }
                        }
                    }
                    //performance optimization ends
                    return true;
                }
            }
        }else if( nullptr != n && NodeType::RESERVATION == n->_type.load( std::memory_order_relaxed ) ){ //try fulfill a waiting dequing thread in queue
            if( head != _head.load( std::memory_order_relaxed ) || tail != _tail.load( std::memory_order_relaxed ) || head->_next.load( std::memory_order_relaxed ) != n ){
                //nodes are invalidated, retry
                continue;
            }
            NodeType expected_head_node_type = NodeType::RESERVATION;
            if( n->_type.compare_exchange_strong( expected_head_node_type, NodeType::BUSY ) ){ //fulfill a dequeing thread
                n->_val = std::move(new_node->_val);
                n->_type.store( NodeType::FULFILLED );
                reclam_hazard<Node>::retire_hazard(new_node);
                new_node = nullptr;
                return true;
            }else{ //unsuccessful fulfillmentm
            }
        }else{
            //performance optimization starts
            Node * current_head = _head.load( std::memory_order_acquire);
            if( nullptr != current_head && current_head->_type.load( std::memory_order_acquire ) == NodeType::SENTINEL ){
                auto guard2 = reclam_hazard<Node>::add_hazard(current_head);
                Node * recycle = current_head;
                Node * current_head_next = current_head->_next.load( std::memory_order_acquire );
                auto guard3 = reclam_hazard<Node>::add_hazard(current_head_next);
                if( current_head_next ){
                    if( current_head_next->_type.load( std::memory_order_acquire ) == NodeType::COMPLETE ){
                        if( _head.compare_exchange_weak( current_head, current_head_next, std::memory_order_acq_rel ) ){
                            current_head_next->_type.store( NodeType::SENTINEL, std::memory_order_release );
                            reclam_hazard<Node>::retire_hazard(recycle);
                            recycle = nullptr;
                        }
                    }
                }
            }
            //performance optimization ends
            std::this_thread::yield();
        }
    }
    assert(false && "unexpected path");
    return false; //shouldn't come here
}
template< typename T >
std::optional<T> queue_lockfree_sync_impl<T, trait_reclamation::hp>::pop_front(){ //pop an item from the head
    //mirror implementation to that of push_back
    Node * new_node = new Node(); //type is RESERVATION if value argument is absent
    while( true ){
        Node * tail = _tail.load( std::memory_order_acquire );

        auto guard = reclam_hazard<Node>::add_hazard(tail);
        
        Node * head = _head.load( std::memory_order_acquire );
        if( nullptr == head || nullptr == tail || head->_type.load( std::memory_order_relaxed ) != NodeType::SENTINEL ){
            std::this_thread::yield();
            continue;
        }
        Node * n = head->_next.load( std::memory_order_relaxed );
        NodeType tail_type = tail->_type.load( std::memory_order_relaxed );
        if( NodeType::SENTINEL == tail_type || NodeType::RESERVATION == tail_type ){ //try enque an item by putting an RESERVATION object in queue
            Node * tail_next = tail->_next.load( std::memory_order_relaxed );

            auto guard2 = reclam_hazard<Node>::add_hazard(tail_next);
            
            if( tail == _tail.load(std::memory_order_relaxed) ){
                if( nullptr != tail_next ){ //tail is invalidated
                    _tail.compare_exchange_weak( tail, tail_next, std::memory_order_relaxed ); //update tail before retry
                }else if( tail->_next.compare_exchange_weak( tail_next, new_node, std::memory_order_relaxed ) ){ //try commit item as the next node
                    _tail.compare_exchange_weak( tail, new_node, std::memory_order_relaxed ); //try update tail after commit
                    //wait for synchronization with dequing thread for the signal that transaction is complete
                    while( new_node->_type.load( std::memory_order_acquire ) != NodeType::FULFILLED ){
                        std::this_thread::yield();
                    }
                    T val(std::move(new_node->_val));
                    new_node->_type.store( NodeType::COMPLETE, std::memory_order_release ); //signal for cleanup for this completed node
                    //performance optimization starts
                    Node * current_head = _head.load( std::memory_order_acquire);

                    auto guard3 = reclam_hazard<Node>::add_hazard(current_head);
                    
                    if( nullptr != current_head && current_head->_type.load( std::memory_order_acquire ) == NodeType::SENTINEL ){
                        Node * recycle = current_head;
                        Node * current_head_next = current_head->_next.load( std::memory_order_acquire );

                        auto guard4 = reclam_hazard<Node>::add_hazard(current_head_next);
                        
                        if( current_head_next ){
                            if( current_head_next->_type.load( std::memory_order_acquire ) == NodeType::COMPLETE ){
                                if( _head.compare_exchange_weak( current_head, current_head_next, std::memory_order_acq_rel ) ){
                                    current_head_next->_type.store( NodeType::SENTINEL, std::memory_order_release );
                                    reclam_hazard<Node>::retire_hazard(recycle);
                                    recycle = nullptr;
                                }
                            }
                        }
                    }
                    //performance optimization ends
                    return std::optional<T>(val);
                }
            }
        }else if( nullptr != n && NodeType::ITEM == n->_type.load( std::memory_order_relaxed ) ){ //try fulfill a waiting enqueing thread in queue
            if( head != _head.load( std::memory_order_relaxed ) || tail != _tail.load( std::memory_order_relaxed ) || head->_next.load( std::memory_order_relaxed ) != n ){
                //nodes are invalidated, retry
                continue;
            }
            NodeType expected_head_node_type = NodeType::ITEM;
            if( n->_type.compare_exchange_strong( expected_head_node_type, NodeType::BUSY ) ){ //fulfill a enqueing thread
                T val(std::move(n->_val));
                n->_type.store( NodeType::FULFILLED);
                reclam_hazard<Node>::retire_hazard(new_node);
                new_node = nullptr;
                return std::optional<T>(val);
            }else{ //unsuccessful fulfillment
            }
        }else{
            //performance optimization starts
            Node * current_head = _head.load( std::memory_order_acquire);

            auto guard2 = reclam_hazard<Node>::add_hazard(current_head);
                                
            if( nullptr != current_head && current_head->_type.load( std::memory_order_acquire ) == NodeType::SENTINEL ){
                Node * recycle = current_head;
                Node * current_head_next = current_head->_next.load( std::memory_order_acquire );

                auto guard3 = reclam_hazard<Node>::add_hazard(current_head_next);
                
                if( current_head_next ){
                    if( current_head_next->_type.load( std::memory_order_acquire ) == NodeType::COMPLETE ){
                        if( _head.compare_exchange_weak( current_head, current_head_next, std::memory_order_acq_rel ) ){
                            current_head_next->_type.store( NodeType::SENTINEL, std::memory_order_release );
                            reclam_hazard<Node>::retire_hazard(recycle);
                            recycle = nullptr;
                        }
                    }
                }
            }
            //performance optimization ends
            std::this_thread::yield();
        }
    }
    assert(false && "unexpected path");
    return std::nullopt; //shouldn't come here
}
template< typename T >
size_t queue_lockfree_sync_impl<T, trait_reclamation::hp>::size(){
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
bool queue_lockfree_sync_impl<T, trait_reclamation::hp>::empty(){
    return size() == 0;
}
template< typename T >
bool queue_lockfree_sync_impl<T, trait_reclamation::hp>::clear(){
    while( !empty() ){
        Node * node = _head.load();
        auto guard = reclam_hazard<Node>::add_hazard(node);
        if( nullptr == node ){
            break;
        }
        if( node->_type.load() == NodeType::ITEM ){
            auto _ = pop_front();
        }else{
            Node * node_next = node->_next.load();
            _head.store( node_next );
            reclam_hazard<Node>::retire_hazard(node);
            node = nullptr;
        }
    }
    return true;
}

