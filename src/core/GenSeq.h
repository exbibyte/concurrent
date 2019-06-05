#ifndef GENSEQ_H
#define GENSEQ_H

//==================================
//
// Summary:      Generates 0-indexed number sequence from an ordered sequence of input classes and assigns each input instance with a number to its member variable "_id_seq"
// Notes:
// Dependencies: N/A
//==================================

namespace GenSeq
{
    template <int... Is>
    struct index {};

    template <int N, int... Is>
    struct gen_seq : gen_seq<N - 1, N - 1, Is...> {};

    template <int... Is>
    struct gen_seq<0, Is...> : index<Is...> {};

    struct Pass { //dummy function needed for executing variadic template function call
	template<typename ...T> Pass(T...) {}
    };

    template < int... Is, typename... Args >
    auto GetSequenceHelper( GenSeq::index<Is...>, Args&... args ) //assigns number to input args
    {
	//Pass{ ( std::cout << Is << std::endl, 1 )... };
	Pass{ ( [&]{
		    args._id_seq = Is;
		    //std::cout << args._id_seq;
	        }(),
	        1
	      )...
	    };
	return true;
    }

    template < template < typename... Args > class Cons, typename... Args >
    auto GenSequence( Args... args ) //generates sequence using Constructor type and input arguments and return a Con type instance with injection class inheritance each assigned an unique sequence id.
    {
	auto sequence = GenSeq::gen_seq<sizeof...(Args)>{};
	GetSequenceHelper( sequence, args... );
	Cons<Args...> _z( args... );
	return _z;
    }   
}

#endif
