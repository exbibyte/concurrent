#ifndef ELIMINATION_ARRAY_HPP
#define ELIMINATION_ARRAY_HPP

#include <vector>
#include <random>
#include <cstring>

#include "exchanger_lockfree.hpp"

template<typename ValType, class RandGen = std::minstd_rand>
class elimination_array {
public:
    
                                                         //visit a slot within [start, end)
                                                    bool visit( ValType & val, size_t start, size_t end );
                                                    bool set_timeout( long duration_us );
                                                         elimination_array(size_t array_size=2) :
                                                             _timeout_us( 30000 ),
                                                             _elim_array(std::vector< exchanger_lockfree<ValType> >(array_size)) {}
                                                         elimination_array(RandGen & g) : _timeout_us( 30000 ), _generator(g) {}
                                                         elimination_array(elimination_array const &) = delete;
                                     elimination_array & operator=(elimination_array const &) = delete;
                                                  size_t array_size() const { return _elim_array.size(); }
private:
              std::vector< exchanger_lockfree<ValType> > _elim_array;
                                                    long _timeout_us;
                                                 RandGen _generator;

};

#include "elimination_array.tpp"

#endif
