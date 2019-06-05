#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>
#include <thread>
#include <functional>
#include <algorithm>
#include <cassert>

#include "combining_tree.hpp"

using namespace std;

int main(){
    int num_threads = 64;
    combining_tree ct(num_threads);
    vector<thread> t(num_threads);
    vector<int> rets(num_threads*2, -1);
    function<void(int,vector<int>&)> increment = [&](int id, vector<int> & arr){
        arr[id] = ct.get_and_increment(id);
        arr[id*2] = ct.get_and_increment(id);
    };
    int thread_id = 0;
    for( auto & i : t ){
	i = thread( increment, thread_id, std::ref(rets) );
	++thread_id;
    }
    for( auto & i : t ){
	i.join();
    }
    sort( rets.begin(), rets.end() );
    cout << "ordered previous return vals: ";
    for_each( rets.begin(), rets.end(), [&](int a){ cout << a << " "; } );
    cout << endl;
    assert( rets.back() == num_threads * 2 - 1);
    return 0;
}
