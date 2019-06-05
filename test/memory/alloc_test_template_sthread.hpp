#include <vector>
#include <cstring>

#include "catch.hpp"

class dummy {
public:
    dummy( int &a, double &b ) : _a(a), _b(b) {}
    dummy( int&& a, double&& b ) : _a(std::move(a)), _b(std::move(b)) {}
    int _a;
    double _b;
};

template< class Allocator >
void test_allocating_freeing(){
    
    size_t block_size = 200'000'000;
    Allocator alc( block_size );
    void * p;
    size_t p_size = 100'000'000;
    bool zeroed = true;
    bool bret = alc.allocating( &p, p_size, zeroed );
    CHECK( bret );

    CHECK( (block_size - p_size) == alc.stat_free_size_total() );
    CHECK( (block_size - p_size) == alc.stat_free_size_largest() );
    CHECK( (block_size - p_size) == alc.stat_free_size_mean() );
    CHECK( 1 == alc.stat_free_count_blocks() );
    CHECK( 1 == alc.stat_lent_count_blocks() );
    
    double err = 0.001;
    CHECK( ((double)0.5 - err) <= alc.stat_free_fraction() );
    CHECK( ((double)0.5 + err) >= alc.stat_free_fraction() );
    
    for( int i = 0; i < p_size; ++i ){
	((char*)p)[i] = 3;
    }

    bret = alc.freeing( p );
    CHECK( bret );

    CHECK( 1 == alc.stat_free_count_blocks() );
    CHECK( 0 == alc.stat_lent_count_blocks() );
    size_t num_blocks = 100;
    std::vector<void*> allocated {};
    for( size_t i = 0; i < num_blocks; ++i ){
	p_size = 1'000'000;
	bret = alc.allocating( &p, p_size, zeroed );
	CHECK( bret );
	allocated.push_back(p);
    }
    CHECK( 1 == alc.stat_free_count_blocks() );
    CHECK( num_blocks == alc.stat_lent_count_blocks() );
    for( auto it = allocated.rbegin(), it_e = allocated.rend(); it!=it_e; ++it ){
	void * to_free = *it;
	bret = alc.freeing( to_free );
	CHECK( bret );
    }
    CHECK( 1 == alc.stat_free_count_blocks() );
    CHECK( 0 == alc.stat_lent_count_blocks() );
    
    bret = alc.clear_internal();
    CHECK( bret );

    size_t free_size_total_after_clear = alc.stat_free_size_total();
    CHECK( free_size_total_after_clear == block_size );
}

template< class Allocator >
void test_newing_deleting(){

    size_t block_size = 200'000'000;
    Allocator alc( block_size );

    size_t num_instances = 100;
    std::vector<dummy*> allocated {};
    for( size_t i = 0; i < num_instances; ++i ){
	dummy * d = alc.template newing<dummy>( nullptr, (int) i, (double) i );
	CHECK( nullptr != d );
	allocated.push_back(d);
    }
    CHECK( 1 == alc.stat_free_count_blocks() );
    CHECK( num_instances == alc.stat_lent_count_blocks() );
    CHECK( (block_size - num_instances*sizeof(dummy)) == alc.stat_free_size_total() );
    
    for( auto it = allocated.rbegin(), it_e = allocated.rend(); it!=it_e; ++it ){
	dummy * to_free = *it;
	bool bret = alc.deleting( to_free );
	CHECK( bret );
    }
    CHECK( 1 == alc.stat_free_count_blocks() );
    CHECK( 0 == alc.stat_lent_count_blocks() );

    CHECK( alc.clear_internal() );

    size_t free_size_total_after_clear = alc.stat_free_size_total();
    CHECK( free_size_total_after_clear == block_size );
}

template< class Allocator >
void test_newing_deleting_placement(){

    size_t block_size = 200'000'000;
    Allocator alc( block_size );

    size_t num_instances = 100;
    std::vector<dummy*> allocated {};
    for( size_t i = 0; i < num_instances; ++i ){
	size_t p_size = sizeof(dummy);
	void * p;
	bool bret = alc.allocating( &p, p_size );
	CHECK( bret );
	dummy * d = alc.template newing<dummy>( p, (int) i, (double) i ); //construct with placement pointer
	CHECK( nullptr != d );
	allocated.push_back(d);
    }
    CHECK( 1 == alc.stat_free_count_blocks() );
    CHECK( num_instances == alc.stat_lent_count_blocks() );
    CHECK( (block_size - num_instances*sizeof(dummy)) == alc.stat_free_size_total() );
    
    for( auto it = allocated.rbegin(), it_e = allocated.rend(); it!=it_e; ++it ){
	dummy * to_free = *it;
	bool bret = alc.deleting( to_free );
	CHECK( bret );
    }
    CHECK( 1 == alc.stat_free_count_blocks() );
    CHECK( 0 == alc.stat_lent_count_blocks() );

    CHECK( alc.clear_internal() );

    size_t free_size_total_after_clear = alc.stat_free_size_total();
    CHECK( free_size_total_after_clear == block_size );
}

template< class Allocator >
void test_invalid_deleting(){

    size_t block_size = 200'000'000;
    Allocator alc( block_size );
    dummy * d = alc.template newing<dummy>( nullptr, 25, 25);
    
    char * e = (char*)d;
    e += block_size;

    bool bret = alc.deleting( (dummy*)e );
    CHECK( false == bret );

    bret = alc.deleting( d );
    CHECK( bret );
    
    CHECK( alc.clear_internal() );

    size_t free_size_total_after_clear = alc.stat_free_size_total();
    CHECK( free_size_total_after_clear == block_size );
}


template< class Allocator >
void test_newing_out_of_memory(){

    size_t block_size = 1;
    Allocator alc( block_size );
    bool caught = false;
    try{
	dummy * d = alc.template newing<dummy>( nullptr, 25, 25);
    }catch( std::bad_alloc & e ){
	caught = true;
    }
    CHECK( caught );
    CHECK( alc.clear_internal() );
    size_t free_size_total_after_clear = alc.stat_free_size_total();
    CHECK( free_size_total_after_clear == block_size );
}
