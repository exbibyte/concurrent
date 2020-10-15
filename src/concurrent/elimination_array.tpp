template< typename ValType, class RandGen>
bool elimination_array<ValType, RandGen>::visit( ValType & val, size_t start, size_t end ){
    assert(start<end);
    std::uniform_int_distribution<int> _distribution(start, std::min(end-1, _elim_array.size()-1));
    int index = _distribution( _generator );
    return _elim_array[index].exchange( val, _timeout_us );
}

template< typename ValType, class RandGen>
bool elimination_array<ValType, RandGen>::set_timeout( long duration_us ){
    _timeout_us = duration_us;
    return true;
}
