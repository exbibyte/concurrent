//based on combining tree section from Art of Multiprocessor Programming
#ifndef COMBINING_TREE_HPP
#define COMBINING_TREE_HPP

#include <vector>
#include <atomic>
#include <mutex>

class probe;

class combining_tree {
public:
    enum eStatus {
	IDLE,
	FIRST,
	SECOND,
	RESULT,
	ROOT,
    };
    class node {
    public:
	int _first;
	int _second;
	int _result;
	node * _parent;
	eStatus _status;
	std::atomic<bool> _locked;
	node() : _locked(false), _parent(nullptr), _status(ROOT), _first(0), _second(0), _result(0) {}
	node( node * p ) : _locked(false), _parent(p), _status(IDLE), _first(0), _second(0), _result(0) {}
	//node methods
	bool precombine();
	int combine( int accumulate );
	int op( int accumulate );
	void distribute( int prior );
    private:
	//helper methods
	void spin_lock_for_access();
	std::mutex _mutex_precomb;
	std::mutex _mutex_comb;
	std::mutex _mutex_op;
	std::mutex _mutex_distr;
    };
    std::vector<node*> _nodes;
    std::vector<node*> _leaves;
    int _width;
    friend class probe;

    combining_tree( int width );
    int get_and_increment(int id);
};

#endif
