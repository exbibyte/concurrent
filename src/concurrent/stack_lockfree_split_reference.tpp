//implementation of split reference lock free stack based on C++ Concurrency in Action Section 7.2
#ifndef STACK_LF_SPLIT_REF_TPP
#define STACK_LF_SPLIT_REF_TPP

#include <atomic>
#include <cstddef>
#include <mutex>

//todo: specialize for reclamation

template< class T, trait_reclamation reclam >
bool stack_lockfree_split_reference_impl<T, reclam>::push( T const & val ){
    //create new external and internal nodes for the associated value
    NodeExternal * new_node = new NodeExternal;
    new_node->_node = new Node( val );
    new_node->_count_external = 1;
    NodeExternal * head = _head.load( std::memory_order_relaxed );
    new_node->_node->_next = head;
    while( !_head.compare_exchange_weak( new_node->_node->_next, new_node, std::memory_order_release, std::memory_order_relaxed ) );
    return true;
}
template< class T, trait_reclamation reclam >
bool stack_lockfree_split_reference_impl<T, reclam>::AcquireNode( NodeExternal * & external_node ){
    //gain ownership of the head
    NodeExternal * temp = new NodeExternal;
    do {
	if( !external_node ){ //empty
	    delete temp;
	    return false;
	}
	temp->_node = external_node->_node;
	temp->_count_external = external_node->_count_external;
	++temp->_count_external;
    } while( !_head.compare_exchange_strong( external_node, temp, std::memory_order_acquire, std::memory_order_relaxed ) );
    if( external_node ){
	delete external_node;
    }
    external_node = temp;
    return true;
}

template< class T, trait_reclamation reclam >
bool stack_lockfree_split_reference_impl<T, reclam>::pop( T & val ){
    NodeExternal * head = _head.load( std::memory_order_relaxed );
    while( true ) {
	if( !AcquireNode( head ) )
	    return false;

	Node * saved_node = head->_node;
	if( !saved_node ){ //empty internal node, so remove the external node from the stack
	    NodeExternal * temp = head;
	    if( _head.compare_exchange_strong( temp, nullptr, std::memory_order_release, std::memory_order_relaxed ) ){
		delete head;
		continue;
	    }
	}
	else if( _head.compare_exchange_strong( head, saved_node->_next, std::memory_order_relaxed ) ){ //take ownership of the head
	    val = saved_node->_val;
	    int count_increase = head->_count_external - 2; //1 for linkage to list, 1 for current thread access
	    if( -count_increase == saved_node->_count_internal.fetch_add( count_increase, std::memory_order_release ) ){ //last reference to node, thus remove it
		if( head ){
		    if( head->_node ){
			delete head->_node;
			head->_node = nullptr;
		    }
		    delete head;
		    head = nullptr;
		}
	    }
	    return true;
	}else if( 1 == saved_node->_count_internal.fetch_sub(1) ){ //other thread took ownership of the head, so decrease internal count and try again; clean up if necessary
	    if( saved_node ){
		saved_node->_count_internal.load( std::memory_order_acquire ); //sync
		delete saved_node;
		saved_node = nullptr;
	    }
	}
    }
    return true;
}

template< class T, trait_reclamation reclam >
size_t stack_lockfree_split_reference_impl<T, reclam>::size() const {
    NodeExternal * current_node = _head.load( std::memory_order_relaxed );
    size_t count = 0;
    while( current_node ){
	Node * internal_node = current_node->_node;
	if( !internal_node ){
	    break;
	}
	++count;
	current_node = internal_node->_next;
    }
    return count;
}

#endif
