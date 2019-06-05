#include <list>

#include "hash_universal.hpp"

template< class K, class V >
hashtable_universal_chain_impl< K, V >::hashtable_universal_chain_impl( size_t table_size ){
    _count_items = 0;
    _table.clear();
    _func_hash_selected = nullptr;
    if( !resize( table_size ) ){
	assert( 0 && "initialization failed." );
    }
}
template< class K, class V >
hashtable_universal_chain_impl< K, V >::~hashtable_universal_chain_impl(){
    for( size_t i = 0; i < _table.size(); ++i ){
    	while( _table[i] ){
    	    remove_hashnode( _table[i] );
    	}
    }
    _table.clear();
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::insert( K const key, V const & value ){
    size_t hashed_key;
    if( !compute_hash( key, hashed_key ) )
	return false;
    hashnode * & head = _table[ hashed_key ];
    hashnode * find = find_hashnode( head, key );
    if( find ){
	find->_val = value;
    }else{
	prepend_hashnode( head, key, value );
	++_count_items;
    }
    return true;
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::find( K const key, V & value ){
    size_t hashed_key;
    if( !compute_hash( key, hashed_key ) )
	return false;
    hashnode * head = _table[ hashed_key ];
    hashnode * find = find_hashnode( head, key );
    if( find ){
	value = find->_val;
	return true;
    }
    return false;
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::erase( K const key ){
    size_t hashed_key;
    if( !compute_hash( key, hashed_key ) )
	return false;
    hashnode * & head = _table[ hashed_key ];
    hashnode * find = find_hashnode( head, key );
    if( find ){
	if( find == head ){
	    remove_hashnode( head );
	}else{
	    remove_hashnode( find );
	}
	--_count_items;
	return true;
    }
    return false;
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::add_hash_func( std::function< size_t( size_t ) > hash_func ){
    _funcs_hash.push_back( hash_func );
    return true;
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::get_hash_func_current( std::function< size_t( size_t ) > & hash_func ){
    if( nullptr == _func_hash_selected )
	return false;

    hash_func = _func_hash_selected;
    return true;
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::resize( size_t size ){
    if( size == get_table_size() || 0 == size ){
	return false;
    }
    //get existing hashed items
    std::list<hashnode*> existing {};
    for( auto it = _table.begin(), it_end = _table.end(); it!=it_end; ++it ){
	hashnode * n = *it;
	while( n ){
	    existing.push_back( n );
	    n = n->_next;
	}
    }
    _table.clear();
    _table.resize( size, nullptr );
    //regenerate hash functions using default generator
    set_default_hash_funcs( size );
    if( !select_random_hash_func() ){
	return false;
    }
    //rehash existing items
    _count_items = 0;
    for( auto it = existing.begin(), it_end = existing.end(); it!=it_end; ++it ){
	insert( (*it)->_key, (*it)->_val );
	delete (*it);
    }
    return true;
}
template< class K, class V >
size_t hashtable_universal_chain_impl< K, V >::get_table_size(){
    return _table.size();
}
template< class K, class V >
double hashtable_universal_chain_impl< K, V >::get_load_factor(){
    if( 0 == get_table_size() ){
	assert( 0 && "table size not valid for load factor calculation." );
	return 0;
    }
    return (double) _count_items / get_table_size();
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::compute_hash( K const key, size_t & hashed_val ){
    hashed_val = _func_hash_selected( (size_t) key );
    return true;
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::select_random_hash_func(){
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
bool hashtable_universal_chain_impl< K, V >::set_default_hash_funcs( size_t const table_size ){
    return hash_universal< K >::generate( table_size, _funcs_hash );
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::prepend_hashnode( hashnode * & node, K const key, V const & val ){
    hashnode * new_node = new hashnode;
    new_node->_key = key;
    new_node->_val = val;
    if( !node ){
	node = new_node;
    }else{
	new_node->_next = node;
	new_node->_prev = node->_prev;
	if( node->_prev ){
	    node->_prev->_next = new_node;
	    node->_prev = new_node;
	}else{
	    node = new_node; //head
	}
    }
    return true;
}
template< class K, class V >
bool hashtable_universal_chain_impl< K, V >::remove_hashnode( hashnode * & node ){
    if( !node )
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
    }else{
	node = node_next;
    }
    return true;
}
template< class K, class V >
typename hashtable_universal_chain_impl< K, V >::hashnode * hashtable_universal_chain_impl
< K, V >::find_hashnode( hashnode * node, K const key ){
    hashnode * current_node = node;
    while( current_node ){
	if( current_node->_key == key ){
	    return current_node;
	}else{
	    current_node = current_node->_next;
	}
    }
    return nullptr;
}
