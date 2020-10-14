template< typename ValType, size_t ArraySize, class RandGen>
bool elimination_array<ValType, ArraySize, RandGen>::visit( ValType & val, size_t start, size_t end ){
    assert(start<end);
    std::uniform_int_distribution<int> _distribution(start, std::min(end-1, ArraySize-1));
    int index = _distribution( _generator );
    return _elim_array[index].exchange( val, _timeout_us );
}

template< typename ValType, size_t ArraySize, class RandGen>
bool elimination_array<ValType, ArraySize, RandGen>::set_timeout( long duration_us ){
    _timeout_us = duration_us;
    return true;
}
