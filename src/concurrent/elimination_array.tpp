template< typename ValType, size_t ArraySize >
bool elimination_array<ValType, ArraySize>::visit( ValType & val, size_t range ){
    if( range >= _size ){
	range = _size - 1;
    }
    std::uniform_int_distribution<int> _distribution(0,range);
    int index = _distribution( _generator );
    return _elim_array[index].exchange( val, _timeout_us );
}

template< typename ValType, size_t ArraySize >
bool elimination_array<ValType, ArraySize>::set_timeout( long duration_us ){
    _timeout_us = duration_us;
    return true;
}
