#include "reclaim_hazard.hpp"

template< typename T >
stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::stack_lockfree_total_simple_impl(){
    _head.store( nullptr );
}
template< typename T >
stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::~stack_lockfree_total_simple_impl(){

    clear();

    assert( _head.load() == nullptr );
    
    reclaim_hazard<Node>::final_deinit();
}
template< typename T>
void stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::thread_deinit(){
    reclaim_hazard<Node>::thread_deinit();
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::push( T const & val ){
    if(auto existing = reclaim_hazard<Node>::new_from_recycled()){
        existing->_val = val;
        existing->_next = nullptr;
        return push_aux(existing);
    }else{
        Node * new_node = new Node( val );
        return push_aux(new_node);
    }
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::push( T && val ){
    if(auto existing = reclaim_hazard<Node>::new_from_recycled()){
        existing->_val = std::move(val);
        existing->_next = nullptr;
        return push_aux(existing);
    }else{
        Node * new_node = new Node( val );
        return push_aux(new_node);
    }
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::try_push( T const & val ){
    Node * n;
    if(auto existing = reclaim_hazard<Node>::new_from_recycled()){
        existing->_val = val;
        existing->_next = nullptr;
        n = existing;
    }else{
        n = new Node( val );
    }

    if(try_push_aux(n)){
        return true;
    }else{
        reclaim_hazard<Node>::retire_hazard(n);
        return false;
    }
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::try_push( T && val ){
    Node * n;
    if(auto existing = reclaim_hazard<Node>::new_from_recycled()){
        existing->_val = std::move(val);
        existing->_next = nullptr;
        n = existing;
    }else{
        n = new Node( val );
    }

    if(try_push_aux(n)){
        return true;
    }else{
        reclaim_hazard<Node>::retire_hazard(n);
        return false;
    }
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::push_aux( Node * new_node ){

    auto guard2 = reclaim_hazard<Node>::add_hazard( new_node );
    
    while(true){

        Node * head = _head.load( std::memory_order_acq_rel );
        
        auto guard = reclaim_hazard<Node>::add_hazard( head );
        
        new_node->_next = head;

        if(_head.compare_exchange_strong( new_node->_next, new_node )){
            break;
        }else{
            std::this_thread::yield();
        }
    }

    return true;
}

template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::try_push_aux( Node * new_node ){

    auto guard2 = reclaim_hazard<Node>::add_hazard( new_node );

    Node * head = _head.load( std::memory_order_acq_rel );
        
    auto guard = reclaim_hazard<Node>::add_hazard( head );
        
    new_node->_next = head;

    if(_head.compare_exchange_strong( new_node->_next, new_node )){
        return true;
    }else{
        return false;
    }
}

template< typename T >
std::optional<T> stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::pop(){

    while(true){

        Node * head = _head.load( std::memory_order_acquire );

        auto guard = reclaim_hazard<Node>::add_hazard( head );

        if(!head){
            return std::nullopt;
        }

        Node * next = head->_next;

        if(_head.compare_exchange_strong( head, next )){

            T val(std::move(head->_val));
    
            reclaim_hazard<Node>::retire_hazard(head);

            return std::optional<T>(val);
    
        }else{
            std::this_thread::yield();
        }
    }

    return std::nullopt;
}

template< typename T >
std::optional<T> stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::try_pop(){

    Node * head = _head.load( std::memory_order_acquire );

    auto guard = reclaim_hazard<Node>::add_hazard( head );

    if(!head){
        return std::nullopt;
    }

    Node * next = head->_next;

    if(_head.compare_exchange_strong( head, next )){

        T val(std::move(head->_val));
    
        reclaim_hazard<Node>::retire_hazard(head);

        return std::optional<T>(val);
    
    }else{
        return std::nullopt;
    }
}

template< typename T >
size_t stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::size() const {
    ///only approximate
    Node * current_node = _head.load( std::memory_order_relaxed );
    size_t count = 0;
    while( current_node ){
        ++count;
        current_node = current_node->_next;
    }
    return count;
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::empty() const {
    return size() == 0;
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::hp>::clear(){
    while( !empty() ){
        auto _  = pop();
    }
    return true;
}
