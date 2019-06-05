#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <cassert>
#include <cstdlib>

#include "hashtable_lock_striped.hpp"
#include "stress_hashtable.hpp"

using namespace std;

int main(){

    hashtable_lock_striped< int, int > hashtable( 200, 0.33 ); //200 buckets, 0.33 lock factor
    unsigned int num_threads = 1000;
    
    stress_hashtable::stress_put_get_int( num_threads, hashtable );

    return 0;
}
