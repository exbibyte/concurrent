#ifndef IDENTITYGEN_H
#define IDENTITYGEN_H

#include <utility>

//==================================
//
// Summary:      Wrapper for a host class to acquire a id
// Notes:
// Dependencies: N/A
//==================================

template< typename T >
class Identity{};

template< typename Host >
class IdentityGen : public Host {
public:
    typedef Host HostType;
    static Identity<Host> _identity;
    int _id_seq;
    int GetIdSeq( Identity<Host> ){ return _id_seq; }
    template<typename... Args>
    // IdentityGen( Args&&... args ) : Host( std::forward<Args>(args)... ) {} //constructor forwarding need to be be modified
    IdentityGen(){}
};


#endif
