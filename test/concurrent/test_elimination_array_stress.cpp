#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include "catch.hpp"
#include "elimination_array.hpp"
#include <functional>

#include <chrono>
#include <thread>

using namespace std;

template< typename ValType, size_t ElimSize >
void stress( long timeout_us, size_t visit_range, size_t num_threads ){
    elimination_array<ValType, ElimSize> elim;
    elim.set_timeout( timeout_us );

    function<void(bool*,int*)> f = [&](bool* b, int * a){
        *b = elim.visit( *a, 0, visit_range );
    };

    vector<int> vals(num_threads);
    for(int i=0; i<vals.size(); ++i){
        vals[i]=i+1;
    }
    bool * rets = new bool[num_threads];
    for(int i=0; i<vals.size(); ++i){
        rets[i] = false;
    }
    vector<thread> ts(num_threads);
    for(size_t i = 0; i < num_threads; ++i ){
        bool * b = &rets[i];
        int * a = &vals[i];
        ts[i] = std::thread( f, b, a );
    }

    for(size_t i = 0; i < num_threads; ++i ){
        ts[i].join();
    }

    unordered_map<int,int> freq;
    for(size_t i = 0; i < num_threads; ++i ){
        if(rets[i]){
            freq[vals[i]]++;
        }
    }
    
    std::cout << "threads joined." << std::endl;

    //check each value has changed
    int count_eliminated = 0;
    for( size_t i = 0; i < num_threads; ++i ){
        count_eliminated += rets[i] ? 1 : 0;
    }

    std::cout << "num thread: " << num_threads << ", size elimnation array: " << ElimSize << ", visit range: " << visit_range << ", timeout: " << timeout_us << ", percent eliminated: " << (double)count_eliminated / num_threads * 100.0 << std::endl;

    for(auto [k,v]: freq){
        assert(k>=0 && k <= num_threads);
        assert(v==1);
    }

    delete [] rets;
}

TEST_CASE( "elimination_array elim size 1, timeout 10ms, thread count 10, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 10000;
        size_t visit_range = 1;
        size_t num_threads = 10;
        stress<int, 1>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 1, timeout 10ms, thread count 100, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 10000;
        size_t visit_range = 1;
        size_t num_threads = 100;
        stress<int, 1>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 1, timeout 10ms, thread count 1000, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 10000;
        size_t visit_range = 1;
        size_t num_threads = 1000;
        stress<int, 1>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 1, timeout 10ms, thread count 10000, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 10000;
        size_t visit_range = 1;
        size_t num_threads = 10000;
        stress<int, 1>( timeout_us, visit_range, num_threads );
    }
}


TEST_CASE( "elimination_array elim size 1, timeout 1ms, thread count 10000, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 1000;
        size_t visit_range = 1;
        size_t num_threads = 10000;
        stress<int, 1>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 10, timeout 1ms, thread count 10000, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 1000;
        size_t visit_range = 10;
        size_t num_threads = 10000;
        stress<int, 10>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 100, timeout 1ms, thread count 10000, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 1000;
        size_t visit_range = 100;
        size_t num_threads = 10000;
        stress<int, 100>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 100, timeout 10ms, thread count 10000, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 10000;
        size_t visit_range = 100;
        size_t num_threads = 10000;
        stress<int, 100>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 100, timeout 100ms, thread count 10000, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 100000;
        size_t visit_range = 100;
        size_t num_threads = 10000;
        stress<int, 100>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 100, timeout 100ms, thread count 1000, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 100000;
        size_t visit_range = 100;
        size_t num_threads = 1000;
        stress<int, 100>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 5, timeout 10ms, thread count 100, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 10000;
        size_t visit_range = 5;
        size_t num_threads = 100;
        stress<int, 5>( timeout_us, visit_range, num_threads );
    }
}

TEST_CASE( "elimination_array elim size 2, timeout 10ms, thread count 100, visit range all", "[elimination_array]" ) {
    size_t loops = 5;
    for( size_t i = 0; i < loops; ++i ){
        long timeout_us = 10000;
        size_t visit_range = 2;
        size_t num_threads = 100;
        stress<int, 2>( timeout_us, visit_range, num_threads );
    }
}
