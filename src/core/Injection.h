#ifndef INJECTION_H
#define INJECTION_H

#include <utility>

//==================================
//
// Summary:      Recursive variadic template for injecting multiple classes into a derived class via multiple inheritance. A constructor argument is passes to each class (intended to be an unique id ). GetIdSeq() function is required by the injection class (intended to retrieve an id from an instance ).
// Notes:
// Dependencies: (Optional: IdentityGen)
//==================================

template< typename... Injections >
class Inject{
};

template< typename First, typename... Injections >
class Inject< First, Injections... > : public Inject< First >, public Inject< Injections... > {
public:
    using Inject< First >::GetIdSeq;
    using Inject< Injections... >::GetIdSeq;
    Inject( First first, Injections... rest ) : Inject< First >( first ), Inject< Injections... >( rest... ) {}
};

template< typename Injection >
class Inject< Injection > : public Injection {
public:
    template< typename... Args>
    Inject( Args&&... args ) : Injection( std::forward<Args>(args)... ) {}  
};

#endif
