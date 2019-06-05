template< class T >
bool hash_universal< T >::generate( size_t const table_size, std::vector< std::function< size_t( size_t ) > > & funcs ){
    if( 0 >= table_size )
	return false;

    funcs.clear();

    constexpr size_t p = 2147483647; //2^31-1 prime number

    //select coefficients for hash function ( ( a * hashed_key + b ) mod p ) mod table_size
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<size_t> distr_a( 1, p-1 ); //a from Zp* = [1,p-1]
    std::uniform_int_distribution<size_t> distr_b( 0, p-1 ); //b from Zp = [0,p-1]

    //create different instances from this set of hash functions Zp* and Zp
    for( size_t i = 0; i < 10; ++i ){
	size_t a = distr_a( engine );
	size_t b = distr_b( engine );
	auto hash_func = std::function< size_t( size_t ) >( [=]( size_t hashed_key ) -> size_t {
		return ( ( a * hashed_key + b ) % p ) % table_size;
	    });
	funcs.push_back( hash_func );
    }

    return true;
}
