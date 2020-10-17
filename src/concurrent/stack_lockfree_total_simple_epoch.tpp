#include "reclam_epoch.hpp"

template< typename T >
stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::stack_lockfree_total_simple_impl(){
    _head.store( nullptr );
}
template< typename T >
stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::~stack_lockfree_total_simple_impl(){

    clear();

    assert( _head.load() == nullptr );
}
template< typename T>
void stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::thread_init(){
    reclam_epoch<Node>::register_thread();
}
template< typename T>
void stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::thread_deinit(){
    reclam_epoch<Node>::unregister_thread();
    reclam_epoch<Node>::deinit_thread();
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::push( T const & val ){
    Node * new_node = new Node( val );
    return push_aux(new_node);
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::push( T && val ){
    Node * new_node = new Node( val );
    return push_aux(new_node);
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::push_aux( Node * new_node ){
    
    while(true){

        auto guard = reclam_epoch<Node>::critical_section();
        
        Node * head = _head.load( std::memory_order_acq_rel );
        
        new_node->_next = head;

        if(_head.compare_exchange_strong( new_node->_next, new_node )){
            break;
        }else{
            guard.done();
            std::this_thread::yield();
        }
    }

    return true;
}
template< typename T >
std::optional<T> stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::pop(){


    while(true){

        auto guard = reclam_epoch<Node>::critical_section();
        
        Node * head = _head.load( std::memory_order_acquire );

        if(!head){
            return std::nullopt;
        }

        Node * next = head->_next;

        if(_head.compare_exchange_strong( head, next )){

            T val(std::move(head->_val));
    
            reclam_epoch<Node>::retire(head);

            return std::optional<T>(val);
    
        }else{
            guard.done();
            std::this_thread::yield();
        }
    }

    return std::nullopt;
}
template< typename T >
size_t stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::size() const {
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
bool stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::empty() const {
    return size() == 0;
}
template< typename T >
bool stack_lockfree_total_simple_impl<T, trait_reclamation::epoch>::clear(){
    while( !empty() ){
        auto _  = pop();
    }
    return true;
}
