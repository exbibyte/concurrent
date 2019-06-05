#include <list>
#include <iostream>
#include <chrono>
#include "hash_universal.hpp"

template< class K, class V >
hashtable_lock_striped_impl< K, V >::hashtable_lock_striped_impl( size_t table_size, double lock_factor ){
    _is_destructing.store( false, std::memory_order_release );
    _is_resizing.store( true, std::memory_order_release );
    _count_items.store( 0, std::memory_order_release );
    _table.clear();
    _func_hash_selected = nullptr;
    _lock_factor = 0.33;
    if( lock_factor <= 0 ){
	assert( 0 && "lock_factor invalid." );	
    }
    if( lock_factor > 1 ){
	lock_factor = 1;
    }
    _lock_factor = lock_factor;
    _is_resizing.store( false, std::memory_order_release );
    if( !resize( table_size ) ){
	assert( 0 && "initialization failed." );
    }
}
template< class K, class V >
hashtable_lock_striped_impl< K, V >::~hashtable_lock_striped_impl(){
    while(true){
	bool resize = false;
	if( _is_resizing.compare_exchange_weak( resize, true, std::memory_order_acq_rel ) ){
	    _is_destructing.store( true, std::memory_order_release );
	    for( auto & i : _locks ){
		while( !i.lock() ){
		    // std::cout <<" here destructing!" <<std::endl;
		}
	    }
	    //enter critical region
	    for( size_t i = 0; i < _table.size(); ++i ){
		hashnode * n = _table[i];
		while( n ){
		    hashnode * n_next = n->_next;
		    delete n;
		    n = n_next;
		}
	    }
	    for( auto & i : _locks ){
		while( !i.unlock() );
	    }
	    break;
	}
	// std::cout << "destruct_waiting...." << std::endl;
    }
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::insert( K const key, V const & value ){
    size_t hashed_key;
    size_t count_lock;
    while( true ){
	if( _is_destructing.load( std::memory_order_acquire ) ){
	    return false;
	}
	if( _is_resizing.load( std::memory_order_acquire ) ){
	    if( std::this_thread::get_id() != _id_resize.load( std::memory_order_acquire ) ){
		continue;
	    }
	}
        count_lock = _lock_count.load( std::memory_order_acquire );
	if( !compute_hash( key, hashed_key ) ){
	    assert( 0 && "hashing key failed." );
	    continue;
	}
	if( _locks[ hashed_key % count_lock ].lock() ){
	    break;
	}
    }
    //entered critical region
    assert( hashed_key < _table.size() && "hashed_key out of table size limti" );
    hashnode * head = _table[ hashed_key % _table.size() ];
    hashnode * find = find_hashnode( head, key );
    if( find ){
	find->_val = value;
    }else{
	hashnode * new_head = prepend_hashnode( head, key, value );
	_table[ hashed_key % _table.size() ] = new_head;
	_count_items.fetch_add(1);
    }
    while( !_locks[ hashed_key % count_lock ].unlock() ){}
    //exited critical region
    return true;
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::find( K const key, V & value ){
    size_t hashed_key;
    size_t count_lock;
    while( true ){
	if( _is_destructing.load( std::memory_order_acquire ) ){
	    return false;
	}
	if( _is_resizing.load( std::memory_order_acquire ) ){
	    continue;
	}
        count_lock = _lock_count.load( std::memory_order_acquire );
	if( !compute_hash( key, hashed_key ) ){
	    assert( 0 && "hashing key failed." );
	    continue;
	}
	if( _locks[ hashed_key % count_lock ].lock() ){
	    break;
	}
    }
    //entered critical region
    assert( hashed_key < _table.size() && "hashed_key out of table size limti" );
    hashnode * head = _table[ hashed_key % _table.size() ];
    hashnode * find = find_hashnode( head, key );
    if( find ){
	value = find->_val;
	while( !_locks[ hashed_key % count_lock ].unlock() ){}
	//exited critical region
	return true;
    }
    
    while( !_locks[ hashed_key % count_lock ].unlock() ){}
    //exited critical region
    return false;
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::erase( K const key ){
    size_t hashed_key;
    size_t count_lock;
    while( true ){
	if( _is_destructing.load( std::memory_order_acquire ) ){
	    return false;
	}
	if( _is_resizing.load( std::memory_order_acquire ) ){
	    continue;
	}
        count_lock = _lock_count.load( std::memory_order_acquire );
	if( !compute_hash( key, hashed_key ) ){
	    assert( 0 && "hashing key failed." );
	    continue;
	}
	if( _locks[ hashed_key % count_lock ].lock() ){
	    break;
	}
    }
    //entered critical region
    assert( hashed_key < _table.size() && "hashed_key out of table size limti" );
    hashnode * head = _table[ hashed_key % _table.size() ];
    hashnode * find = find_hashnode( head, key );
    if( find ){
	if( find == head ){
	    hashnode * n = find->_next;
	    remove_hashnode( find );
	    _table[ hashed_key % _table.size() ] = n;
	}else{
	    remove_hashnode( find );
	}
	_count_items.fetch_sub(1);

	while( !_locks[ hashed_key % count_lock ].unlock() ){}
	//exited critical region
	return true;
    }

    //at this point no key is found, so just release the lock
    while( !_locks[ hashed_key % count_lock ].unlock() ){}
    //exited critical region
    return false;
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::add_hash_func( std::function< size_t( size_t ) > hash_func ){
    _funcs_hash.push_back( hash_func );
    return true;
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::get_hash_func_current( std::function< size_t( size_t ) > & hash_func ){
    if( nullptr == _func_hash_selected )
	return false;

    hash_func = _func_hash_selected;
    return true;
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::resize( size_t new_size ){
    size_t old_size = get_table_size();
    if( new_size == old_size || 0 == new_size ){
	return false;
    }
    bool resize = false;
    _id_resize.store( std::this_thread::get_id(), std::memory_order_release );
    if( _is_resizing.compare_exchange_strong( resize, true, std::memory_order_acq_rel ) ){

	if( old_size != get_table_size() ){
	    _is_resizing.store( false, std::memory_order_release );
	    return false;
	}
	for( auto & i : _locks ){
	    while( !i.lock() ){
		// std::cout << "locking" << std::endl;
	    }
	}
	//inside the critial region at this point

	size_t new_lock_count = (size_t )( new_size * _lock_factor );
	if( new_lock_count > new_size ){
	    new_lock_count = new_size;
	}
	if( 0 == new_lock_count ){
	    new_lock_count = 1;
	}
	
	_locks.clear();
	_locks.resize( new_lock_count );
	_lock_count.store(new_lock_count, std::memory_order_release );

	//get existing hashed items
	std::list<hashnode*> existing {};
	for( auto it = _table.begin(), it_end = _table.end(); it!=it_end; ++it ){
	    hashnode * n = *it;
	    while( n ){
		existing.push_back( n );
		n = n->_next;
	    }
	}
	// std::cout <<" here resize!" <<std::endl;
	_table.clear();
	_table.resize( new_size, nullptr );
	//regenerate hash functions using default generator
	set_default_hash_funcs( new_size );
	if( !select_random_hash_func() ){
	    assert( 0 && "selecting random hash function failed." );
	    return false;
	}
	//rehash existing items
	_count_items.store( 0, std::memory_order_release );
	for( auto it = existing.begin(), it_end = existing.end(); it!=it_end; ++it ){
	    insert( (*it)->_key, (*it)->_val );
	    delete (*it);
	}

	_is_resizing.store( false, std::memory_order_release );
	return true;
    }
    return false;
}
template< class K, class V >
size_t hashtable_lock_striped_impl< K, V >::get_table_size(){
    return _table.size();
}
template< class K, class V >
double hashtable_lock_striped_impl< K, V >::get_load_factor(){
    if( 0 == get_table_size() ){
	assert( 0 && "table size not valid for load factor calculation." );
	return 0;
    }
    return (double) _count_items.load(std::memory_order_relaxed) / get_table_size();
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::compute_hash( K const key, size_t & hashed_val ){
    hashed_val = _func_hash_selected( (size_t) key );
    return true;
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::select_random_hash_func(){
    if( _funcs_hash.empty() ){
	return false;
    }
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<int> distribution(0, _funcs_hash.size()-1 );
    int selected = distribution(engine);
    _func_hash_selected = _funcs_hash[selected];
    return true;
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::set_default_hash_funcs( size_t const table_size ){
    return hash_universal< K >::generate( table_size, _funcs_hash );
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::quiesce(){
    for( auto & i : _locks ){
	while( !i.is_free() ){}
    }
}
template< class K, class V >
typename hashtable_lock_striped_impl< K, V >::hashnode * hashtable_lock_striped_impl< K, V >::prepend_hashnode( hashnode * node, K const key, V const & val ){
    //assumed to have locked the hash bucket
    hashnode * new_node = new hashnode;
    new_node->_key = key;
    new_node->_val = val;
    new_node->_prev = nullptr;
    new_node->_next = nullptr;
    if( nullptr != node ){
	new_node->_next = node;
	new_node->_prev = node->_prev;
	if( nullptr != node->_prev ){
	    node->_prev->_next = new_node;
	}
	node->_prev = new_node;
    }
    return new_node;
}
template< class K, class V >
bool hashtable_lock_striped_impl< K, V >::remove_hashnode( hashnode * & node ){
    //assumed to have locked the hash bucket
    if( nullptr == node )
	return false;
    hashnode * node_prev = node->_prev;
    hashnode * node_next = node->_next;
    delete node;
    node = nullptr;
    if( node_next ){
	node_next->_prev = node_prev;
    }
    if( node_prev ){
	node_prev->_next = node_next;
    }
    return true;
}
template< class K, class V >
typename hashtable_lock_striped_impl< K, V >::hashnode * hashtable_lock_striped_impl
< K, V >::find_hashnode( hashnode * node, K const key ){
    //assumed to have locked the hash bucket
    hashnode * current_node = node;
    while( nullptr != current_node ){
	if( current_node->_key == key ){
	    return current_node;
	}else{
	    // std::cout << "thread " << std::this_thread::get_id() << " here!" <<std::endl;
	    current_node = current_node->_next;   
	}
    }
    return nullptr;
}
