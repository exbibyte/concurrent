#ifndef LOCK_READER_WRITER_SYNC_HPP
#define LOCK_READER_WRITER_SYNC_HPP

#include <thread>
#include <atomic>
#include <cassert>
#include <iostream>

#include "ILock.hpp"

class lock_reader_writer_sync_impl {
public:
    lock_reader_writer_sync_impl(){
	_is_writing.store( false, std::memory_order_release );
	_count_access.store( 0, std::memory_order_release );
    }
    lock_reader_writer_sync_impl(lock_reader_writer_sync_impl const & ){
	_is_writing.store( false, std::memory_order_release );
	_count_access.store( 0, std::memory_order_release );
    }
    bool lock( lock_access_type t ){
	if( lock_access_type::READ == t ){
	    while(true){
		if( !_is_writing.load( std::memory_order_acquire ) ){
		    _count_access.fetch_add( 1 );
		    return true;
		}
	    }
	}else{ //WRITE
	    while(true){
		bool expected_is_writing = false;
		if( _is_writing.compare_exchange_weak( expected_is_writing, true, std::memory_order_acq_rel ) ){
		    while( true ){
			size_t expected_count_access = 0;
			if( _count_access.compare_exchange_weak( expected_count_access, 1, std::memory_order_acq_rel ) ){
			    //at this point, thread gains access to resource
			    return true;
			}
		    }
		}
	    }
	}
    }
    bool unlock( lock_access_type t ){
	if( lock_access_type::READ == t){
	    _count_access.fetch_sub( 1 );
	    return true;
	}else{ //WRITE
	    while(true){
		bool expected_is_writing = true;
		if( _is_writing.compare_exchange_weak( expected_is_writing, false, std::memory_order_acq_rel ) ){
		    //at this point, thread release access to resource
		    _count_access.fetch_sub( 1 );
		    return true;
		}
	    }
	}
    }
    bool is_free() const{
	return count() == 0;
    }
    size_t count() const{
	return _count_access.load( std::memory_order_acquire );
    }
private:
    std::atomic<size_t> _count_access;
    std::atomic<bool> _is_writing;
};

using lock_reader_writer_sync = ILock< lock_reader_writer_sync_impl, trait_lock_method::synchronous >;

#endif
