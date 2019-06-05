#ifndef LOCK_RECURSIVE_TOTAL_HPP
#define LOCK_RECURSIVE_TOTAL_HPP

#include <thread>
#include <atomic>

#include <ILock.hpp>

class lock_recursive_total_impl {
public:
    lock_recursive_total_impl(){
	_count = 0;
	_access.store( true, std::memory_order_release );
    }
    lock_recursive_total_impl(lock_recursive_total_impl const & ){
	_count = 0;
	_access.store( true, std::memory_order_release );			  
    }
    bool lock(){
	bool access = true;
	if( _access.compare_exchange_weak( access, false, std::memory_order_acq_rel ) ){
	    bool access_release = true;
	    if( 0 == _count ){
		_id = std::this_thread::get_id();
		++_count;
		_access.store( access_release, std::memory_order_release );
		return true;
	    }else if( _id == std::this_thread::get_id() ){
		++_count;
		_access.store( access_release, std::memory_order_release );
		return true;
	    }else{
		_access.store( access_release, std::memory_order_release );
		return false;
	    }
	}
	return false;
    }
    bool unlock(){
	bool access = true;
	if( _access.compare_exchange_weak( access, false, std::memory_order_acq_rel ) ){
	    bool access_release = true;
	    if( 0 == _count ){
		_access.store( access_release, std::memory_order_release );
		return true;
	    }else if( _id == std::this_thread::get_id() ){
		--_count;
		_access.store( access_release, std::memory_order_release );
		return true;
	    }else{
		_access.store( access_release, std::memory_order_release );
		return false;
	    }
	}
	return false;
    }
    bool is_free() const{
	return _count == 0;
    }
    size_t count() const{
	return _count;
    }
private:
    std::thread::id _id;
    size_t _count;
    std::atomic<bool> _access;
};

using lock_recursive_total = ILock< lock_recursive_total_impl, trait_lock_method::total >;

#endif
