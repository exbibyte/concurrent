//==================================
//
// Summary:      Sequence Generator, Identity Generator, Injection Test
// Notes:
// Dependencies: N/A
//==================================
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "Injection.h"
#include "GenSeq.h"
#include "IdentityGen.h"
#include "GetIndex.h"

#include <map>
#include <vector>
#include <tuple>
#include <utility>
#include <set>
#include <iostream>
#include <string>

using namespace std;
using namespace GenSeq; 
using namespace GetIndex;

class A {
public:
    bool TestActionA(){
	cout << "action" << endl;
	return true;
    }
};

class B {
public:
    bool TestActionB(){
	cout << "action B" << endl;
	return true;
    }
};

class C {
public:
};


TEST_CASE( "GenSeq, IdentityGen, Injection", "[ALL][Inject][GenSeq]" ) {
    SECTION( "GenSeq, IdentityGen, Injection" ){

	auto _z = GenSequence< Inject >( IdentityGen<A>(), IdentityGen<B>(), IdentityGen<C>() );
	bool bRet = _z.TestActionA();
	bRet &= _z.TestActionB();
	CHECK( true == bRet );

	cout << _z.GetIdSeq(IdentityGen<A>::_identity) << _z.GetIdSeq(IdentityGen<B>::_identity) << _z.GetIdSeq(IdentityGen<C>::_identity) << endl;
	int id_A = _z.GetIdSeq(IdentityGen<A>::_identity);
	int id_B = _z.GetIdSeq(IdentityGen<B>::_identity);
	int id_C = _z.GetIdSeq(IdentityGen<C>::_identity);
	CHECK( 0 == id_A );
	CHECK( 1 == id_B );
	CHECK( 2 == id_C );
    }
}
TEST_CASE( "get_index", "[ALL][GetIndex]" ) {
    SECTION( "get_index" ){
	int id_char = get_index<char, char, int, void>::value;
	int id_int = get_index<int, char, int, void>::value;
        int id_void = get_index<void, char, int, void>::value;
	CHECK( 0 == id_char );
	CHECK( 1 == id_int );
	CHECK( 2 == id_void );
    }
}
