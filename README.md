# concurrent
multithreaded programming in C++, experimental..

They are based on references such as: C++ Concurrency in Action, Art of Multiprocessor Programming, Hands-On Concurrency with Rust.
  
# core implementations
  - combining tree
  - lock striped hashtable (chained, universal hashing)
  - lockfree list
    - total
  - lockfree queue
    - total
    - synchronous
  - lockfree stack
    - total
    - split reference
    - partial elimination
  - elimination array
  - other primitves
    - locks
      - blocking
      - recursive
      - reader-writer
    - lockfree exchanger

# work in progress
  - implement safe memory reclamation strategies
    - hazard pointer
    - epoch (simplify interface/usage)
	- quiescent state (todo)
	- hyaline (todo)
  - add working memory reclamation strategies in concurrent datastructures (in progress)

# currently supported implementations with safe memory reclamation:
  - lockfree queue
    - queue_lockfree_total with hazard pointer / epoch based reclamation
  - lockfree stack
    - stack_lockfree_total_simple with hazard pointer / epoch based reclamation
    - stack_lockfree_elim with hazard pointer

# other notes
  - currently uses C++1z
  - release build: make DEBUG=0 all

# sample usages
    -lockfree queue w/ hazard pointer:
```
#include "queue_lockfree_total.hpp"
#include "threadwrap.hpp"
#include "reclam_hazard.hpp"
//...
    using container_type = queue_lockfree_total<int, trait_reclamation::hp>;
    container_type queue;
    unsigned int num_threads = std::thread::hardware_concurrency()/2;
    size_t nums = num_threads * 3000000;
    
    auto f = [&](){
            size_t count = queue.size();
            CHECK( 0 == count );
            int val = 5;
            queue.put(val);
            count = queue.size();
            CHECK( 1 == count );

            auto ret = queue.get();
            count = queue.size();
            CHECK( 0 == count );
            CHECK( ret );
            CHECK( 5 == *ret );
    };
    threadwrap::this_thread_run<container_type::mem_reclam>(f);

    //spawn some threads
    vector<threadwrap> threads( num_threads );
    vector<threadwrap> threads2( num_threads );    
    for( int i = 0; i < num_threads; ++i ){
        threads[i] = threadwrap( [ &, i ](){
                int val = nums/num_threads*i;
                for( int j = 0; j < nums/num_threads; ++j ){
                    queue.put( val + j ); //blocking put
                }
            }, identity<container_type::mem_reclam>() );
    }

    for( int i = 0; i < num_threads; ++i ){
        threads2[i] = threadwrap( [&](){
                for( int j = 0; j < nums/num_threads; ++j ){
                    if(auto ret = queue.get()){ //non-blocking get
                        //...
                    } 
                }
            }, identity<container_type::mem_reclam>() );
    }
  
    for( auto & i : threads ){
        i.join();
    }
    for( auto & i : threads2 ){
        i.join();
    }
```    