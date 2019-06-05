#include <cstring>
#include <list>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>

#include "combining_tree.hpp"

using namespace std;

combining_tree::combining_tree( int width ){
    //round width to next highest power of two
    if( width < 1 )
	width = 2;

    int rounded = 2;
    while( rounded < width ){
	rounded <<= 1;
    }
    width = rounded;
    _width = width;
    _leaves.resize(width/2); //# leaves = width/2
    _nodes.resize( _leaves.size() * 2 - 1 ); //# nodes = leaves * 2 - 1
    //allocate nodes of the tree
    _nodes[0] = new node; //root
    for( size_t i = 1; i < _nodes.size(); ++i ){
	_nodes[i] = new node( _nodes[(i-1)/2] );
    }
    //initialize leaves (thread entry points) to certain nodes
    size_t offset_leaves = _nodes.size() - _leaves.size();
    for( size_t i = 0; i < _leaves.size(); ++i ){
	_leaves[i] = _nodes[ offset_leaves + i ];
    }
}
void combining_tree::node::spin_lock_for_access(){
    bool is_locked = false;
    while( !_locked.compare_exchange_weak( is_locked, true ) ){
	is_locked = false;
	this_thread::sleep_for(chrono::milliseconds(1));
    }
}
bool combining_tree::node::precombine(){
    
    //compute stop node for the current thread

    lock_guard<std::mutex> lck( _mutex_precomb );

    spin_lock_for_access();

    switch( _status ){
    case IDLE: //current thread is an active thread at current node
    {
	_status = FIRST;
	_locked.store(false); //release node access
	return true;
    }
    break;
    case FIRST: //current thread is a passive thread at current node
    {
	_status = SECOND;
	_locked.store(true); //lock at prevent active thread from premature combine
	return false;
    }
    break;
    case ROOT:
    {
	_locked.store(false); //release node access
	return false;
    }
    break;
    default:
	throw( "unexpected status" );
    }
    return false; //should not get here
}
int combining_tree::node::combine( int accumulate ){
    //active thread combine values deposited at node (possibly by passive thread)

    lock_guard<std::mutex> lck( _mutex_comb );
	
    spin_lock_for_access();

    _first = accumulate; //deposit accumulate value in current node by this active thread

    switch( _status ){
    case FIRST: //no contention with another thread at current node
    {
	return _first;
    }
    break;
    case SECOND: //there is another passive thread for current node
    {
	return _first + _second;
    }
    break;
    default:
	throw( "unexpected status" );
    }
    return -1; //should not get here
}
int combining_tree::node::op( int accumulate ){
    //saves the accumulated value for this node and wait for value returned by highest nodes or just return if it's root

    lock_guard<std::mutex> lck( _mutex_op );
    
    switch( _status ){
    case ROOT:
    {
	int prior = _result;
	_result += accumulate;
	return prior;
    }
    break;
    case SECOND:
    {
	_second = accumulate;
	_locked.store(false); //synchronize with active thread trying to do a combine
	while( RESULT != _status ){
            //wait for synchronization initiated by a distributing thread for current node
	    this_thread::sleep_for( chrono::milliseconds(1) );
	}
	_status = IDLE;
	_locked.store(false);
	return _result; //return result deposited by another active thread for current node
    }
    break;
    default:
	throw( "unexpected status" );
    }
    return -1; //should not get here
}
void combining_tree::node::distribute( int prior ){
    //return value back down the tree by this active thread

    lock_guard<std::mutex> lck( _mutex_distr );
	
    switch( _status ){
    case FIRST:
    {
	_status = IDLE;
	_locked.store(false);
    }
    break;
    case SECOND:
    {
	_result = prior + _first; //sum value brought from up the tree with active thread's sub-branch accumulation
	_status = RESULT;
    }
    break;
    default:
	throw( "unexpected status" );
    }
}
int combining_tree::get_and_increment(int id){
    if( id < 0 || id >= _width )
	throw( "out of expected id range" );

    //precombine phase
    node * start = _leaves[id/2];
    node * n = start;
    while( n->precombine() ){
	n = n->_parent;
    }
    node * stop = n;

    //combine phase
    n = start;
    int accumulate = 1;
    list<node*> path {};
    while( n != stop ){
	accumulate = n->combine( accumulate );
	path.push_back(n);
	n = n->_parent;
    }

    //op phase
    int prior;
    prior = stop->op( accumulate );

    //distribute phase
    while( !path.empty() ){
	n = path.back();
	path.pop_back();
	n->distribute(prior);
    }

    //return the previous value at root node to the current thread
    return prior;
}
