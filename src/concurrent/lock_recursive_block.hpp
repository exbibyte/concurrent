#ifndef LOCK_RECURSIVE_BLOCK_HPP
#define LOCK_RECURSIVE_BLOCK_HPP

#include <thread>
#include <atomic>

#include "ILock.hpp"

class lock_recursive_block_impl {
public:
    lock_recursive_block_impl(){
	_count.store( 0, std::memory_order_release );
	_access.store( true, std::memory_order_release );
    }
    lock_recursive_block_impl(lock_recursive_block_impl const & ){
	_count.store( 0, std::memory_order_release );
	_access.store( true, std::memory_order_release );			  
    }
    bool lock(){
	while( true ){
	    bool access = true;
	    if( _access.compare_exchange_weak( access, false, std::memory_order_acq_rel ) ){
		bool access_release = true;
		if( 0 == _count.load( std::memory_order_acquire ) ){
		    _id = std::this_thread::get_id();
		    _count++;
		    _access.store( access_release, std::memory_order_release );
		    return true;
		}else if( _id == std::this_thread::get_id() ){
		    _count++;
		    _access.store( access_release, std::memory_order_release );
		    return true;
		}else{
		    _access.store( access_release, std::memory_order_release );
		    //retry
		}
	    }
	}
    }
    bool unlock(){
	while( true ){
	    bool access = true;
	    if( _access.compare_exchange_weak( access, false, std::memory_order_acq_rel ) ){
		bool access_release = true;
		if( 0 == _count.load( std::memory_order_acquire ) ){
		    _access.store( access_release, std::memory_order_release );
		    return true;
		}else if( _id == std::this_thread::get_id() ){
		    _count--;
		    _access.store( access_release, std::memory_order_release );
		    return true;
		}else{
		    _access.store( access_release, std::memory_order_release );
		    //retry
		}
	    }
	}
    }
    bool is_free() const{
	return count();
    }
    size_t count() const{
	return _count.load( std::memory_order_acquire );
    }
private:
    std::thread::id _id;
    std::atomic<size_t> _count;
    std::atomic<bool> _access;
};

using lock_recursive_block = ILock< lock_recursive_block_impl, trait_lock_method::synchronous >;

#endif
