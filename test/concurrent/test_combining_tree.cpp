#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <set>
#include <chrono>
#include <thread>
#include <functional>
#include <algorithm>

#include "catch.hpp"

#include "combining_tree.hpp"

using namespace std;

class probe {
public:
    int probe_impl( combining_tree * t, std::function<int(combining_tree *)> f ){
	return f( t );
    }
};

TEST_CASE( "combining_tree initialization width 1", "[combtree]" ) {
    probe p;
    combining_tree ct(1);
    function<int(combining_tree*)> probe_size_nodes = [](combining_tree * t){ return t->_nodes.size(); };
    function<int(combining_tree*)> probe_size_leaves = [](combining_tree * t){ return t->_leaves.size(); };
    int size_nodes = p.probe_impl( &ct, probe_size_nodes );
    int size_leaves = p.probe_impl( &ct, probe_size_leaves );
    CHECK( 1 == size_nodes );
    CHECK( 1 == size_leaves );
}
TEST_CASE( "combining_tree initialization width 2", "[combtree]" ) {
    probe p;
    combining_tree ct(2);
    function<int(combining_tree*)> probe_size_nodes = [](combining_tree * t){ return t->_nodes.size(); };
    function<int(combining_tree*)> probe_size_leaves = [](combining_tree * t){ return t->_leaves.size(); };
    int size_nodes = p.probe_impl( &ct, probe_size_nodes );
    int size_leaves = p.probe_impl( &ct, probe_size_leaves );
    CHECK( 1 == size_nodes );
    CHECK( 1 == size_leaves );
}
TEST_CASE( "combining_tree initialization width 3", "[combtree]" ) {
    probe p;
    combining_tree ct(3);
    function<int(combining_tree*)> probe_size_nodes = [](combining_tree * t){ return t->_nodes.size(); };
    function<int(combining_tree*)> probe_size_leaves = [](combining_tree * t){ return t->_leaves.size(); };
    int size_nodes = p.probe_impl( &ct, probe_size_nodes );
    int size_leaves = p.probe_impl( &ct, probe_size_leaves );
    CHECK( 3 == size_nodes );
    CHECK( 2 == size_leaves );
}
TEST_CASE( "combining_tree initialization width 4", "[combtree]" ) {
    probe p;
    combining_tree ct(4);
    function<int(combining_tree*)> probe_size_nodes = [](combining_tree * t){ return t->_nodes.size(); };
    function<int(combining_tree*)> probe_size_leaves = [](combining_tree * t){ return t->_leaves.size(); };
    int size_nodes = p.probe_impl( &ct, probe_size_nodes );
    int size_leaves = p.probe_impl( &ct, probe_size_leaves );
    CHECK( 3 == size_nodes );
    CHECK( 2 == size_leaves );
}
TEST_CASE( "combining_tree initialization width 6", "[combtree]" ) {
    probe p;
    combining_tree ct(6);
    function<int(combining_tree*)> probe_size_nodes = [](combining_tree * t){ return t->_nodes.size(); };
    function<int(combining_tree*)> probe_size_leaves = [](combining_tree * t){ return t->_leaves.size(); };
    int size_nodes = p.probe_impl( &ct, probe_size_nodes );
    int size_leaves = p.probe_impl( &ct, probe_size_leaves );
    CHECK( 7 == size_nodes );
    CHECK( 4 == size_leaves );
}
TEST_CASE( "combining_tree count 0 to 7", "[combtree]" ) {
    int num_threads = 8;
    int width = 8;
    combining_tree ct(width);
    vector<thread> t(num_threads);
    vector<int> rets(num_threads, -1);
    function<void(int,int&)> increment = [&](int id, int & ret){ ret = ct.get_and_increment(id); };
    int thread_id = 0;
    for( auto & i : t ){
	i = thread( increment, thread_id, std::ref(rets[thread_id]) );
	++thread_id;
    }
    for( auto & i : t ){
	i.join();
    }
    sort( rets.begin(), rets.end() );
    vector<int> expected;
    expected.resize(num_threads);
    int count = 0;
    for( auto & i : expected ){
	i = count;
	++count;
    }
    bool check = rets == expected;
    CHECK( check );
    //check resource locks of all nodes
    size_t num_internal_nodes = distance( ct._nodes.begin(), ct._nodes.end() );
    vector<bool> node_locks(num_internal_nodes, true);
    count = 0;
    for_each( ct._nodes.begin(), ct._nodes.end(),
	      [&count,&node_locks](auto n){
		  bool lock_status = n->_locked.load();
		  node_locks[count] = lock_status;
		  ++count;
	      });
    vector<bool> expected_node_locks(num_internal_nodes,false);
    check = node_locks == expected_node_locks;
    CHECK( check );
}
