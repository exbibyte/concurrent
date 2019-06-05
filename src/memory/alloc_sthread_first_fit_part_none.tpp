#include <new>

template< class T , class... Args >
T * alloc_sthread_first_fit_part_none_impl::newing( void * p, Args&&... args ){
    if( nullptr == p ){
	if( !allocating( &p, sizeof(T) ) ){
	    //allocation failed
	    std::bad_alloc bad_alloc;
	    throw bad_alloc;
	    return nullptr;
	}
    }
    void * ptr = new( p ) T( std::forward<Args>(args)... ); //constructor via placement
    return static_cast<T*>( ptr );
}
template< class T >
bool alloc_sthread_first_fit_part_none_impl::deleting( T * p ){
    if( nullptr == p ){
	return false;
    }
    p->~T(); //call destructor
    return freeing( (void *) p ); //reclaim memory
}
